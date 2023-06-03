#include <gtest/gtest.h>

#include <deadfood/database.hh>

#include <deadfood/lex/lex.hh>

#include <deadfood/parse/create_table_parser.hh>
#include <deadfood/parse/drop_table_parser.hh>
#include <deadfood/parse/insert_parser.hh>
#include <deadfood/parse/select_parser.hh>
#include <deadfood/parse/update_parser.hh>
#include <deadfood/parse/delete_parser.hh>

#include <deadfood/exec/create_table.hh>
#include <deadfood/exec/drop_table.hh>
#include <deadfood/exec/insert_into.hh>
#include <deadfood/exec/select.hh>
#include <deadfood/exec/update.hh>
#include <deadfood/exec/delete.hh>

namespace deadfood::tests {

std::optional<std::pair<std::unique_ptr<scan::IScan>, std::vector<std::string>>>
ProcessQueryInternal(Database& db, const std::string& query) {
  auto tokens = lex::Lex(query);
  if (IsKeyword(tokens[0], lex::Keyword::Create)) {  // create table query
    const auto q = parse::ParseCreateTableQuery(tokens);
    exec::ExecuteCreateTableQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Drop)) {  // drop table query
    const auto q = parse::ParseDropTableQuery(tokens);
    exec::ExecuteDropTableQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Update)) {  // update query
    const auto q = parse::ParseUpdateQuery(tokens);
    exec::ExecuteUpdateQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Delete)) {  // delete query
    const auto q = parse::ParseDeleteQuery(tokens);
    exec::ExecuteDeleteQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Insert)) {  // insert query
    const auto q = parse::ParseInsertQuery(tokens);
    exec::ExecuteInsertQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Select)) {  // select query
    const auto q = parse::ParseSelectQuery(tokens);
    return exec::ExecuteSelectQuery(db, q);
  }
  return std::nullopt;
}

TEST(CreateInsertSelectSingleRow, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (a INT, b FLOAT, c DOUBLE, d "
                       "BOOLEAN, e VARCHAR(10))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES (1, 42.0, 55.0, 0, 'test')");
  auto result = ProcessQueryInternal(db, "SELECT a, b, c, d, e FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("a"), core::FieldVariant(static_cast<int>(1)));
  ASSERT_EQ(scan->GetField("b"), core::FieldVariant(static_cast<float>(42)));
  ASSERT_EQ(scan->GetField("c"), core::FieldVariant(static_cast<double>(55)));
  ASSERT_EQ(scan->GetField("d"), core::FieldVariant(static_cast<bool>(false)));
  ASSERT_EQ(scan->GetField("e"),
            core::FieldVariant(static_cast<std::string>("test")));
  ASSERT_FALSE(scan->Next());
}

TEST(CreateInsertDelete, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (a INT, b FLOAT, c DOUBLE, d "
                       "BOOLEAN, e VARCHAR(10))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES (1, 42.0, 55.0, 0, 'test')");
  ProcessQueryInternal(db, "DROP TABLE test_tbl");
  ASSERT_THROW(ProcessQueryInternal(db, "SELECT a, b, c, d, e FROM test_tbl"),
               std::runtime_error);
}

TEST(PrimaryKeyInsertFail, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (a INT PRIMARY KEY, b FLOAT)");
  ASSERT_THROW(ProcessQueryInternal(db, "INSERT INTO test_tbl (b) VALUES (5)"),
               std::runtime_error);
}

TEST(PrimaryKeyInsertFailDup, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (a INT PRIMARY KEY, b FLOAT)");
  ASSERT_THROW(
      ProcessQueryInternal(db, "INSERT INTO test_tbl VALUES (1, 5), (1, 6)"),
      std::runtime_error);
}

TEST(PrimaryKeyInsertGood, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (a INT PRIMARY KEY, b FLOAT)");
  ProcessQueryInternal(db, "INSERT INTO test_tbl VALUES (1, 5), (2, 6)");
  auto result = ProcessQueryInternal(db, "SELECT a, b  FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("a"), core::FieldVariant(static_cast<int>(1)));
  ASSERT_EQ(scan->GetField("b"), core::FieldVariant(static_cast<float>(5)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("a"), core::FieldVariant(static_cast<int>(2)));
  ASSERT_EQ(scan->GetField("b"), core::FieldVariant(static_cast<float>(6)));
  ASSERT_FALSE(scan->Next());
}

TEST(NotNullFail, db) {
  Database db;
  ProcessQueryInternal(db, "CREATE TABLE test_tbl (a INT NOT NULL, b FLOAT)");
  ASSERT_THROW(
      ProcessQueryInternal(db, "INSERT INTO test_tbl VALUES (1, 5), (NULL, 6)"),
      std::runtime_error);
}

TEST(NullGood, db) {
  Database db;
  ProcessQueryInternal(db, "CREATE TABLE test_tbl (a INT, b FLOAT)");
  ProcessQueryInternal(db, "INSERT INTO test_tbl VALUES (1, 5), (NULL, 6)");
  auto result = ProcessQueryInternal(db, "SELECT a, b  FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("a"), core::FieldVariant(static_cast<int>(1)));
  ASSERT_EQ(scan->GetField("b"), core::FieldVariant(static_cast<float>(5)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("a"), core::FieldVariant(core::null_t{}));
  ASSERT_EQ(scan->GetField("b"), core::FieldVariant(static_cast<float>(6)));
  ASSERT_FALSE(scan->Next());
}

TEST(UniqueInsertGood, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (a VARCHAR(10) UNIQUE, b FLOAT)");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl VALUES ('123', 5), ('555', 6)");
  auto result = ProcessQueryInternal(db, "SELECT a, b  FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("a"),
            core::FieldVariant(static_cast<std::string>("123")));
  ASSERT_EQ(scan->GetField("b"), core::FieldVariant(static_cast<float>(5)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("a"),
            core::FieldVariant(static_cast<std::string>("555")));
  ASSERT_EQ(scan->GetField("b"), core::FieldVariant(static_cast<float>(6)));
  ASSERT_FALSE(scan->Next());
}

TEST(UniqueInsertFail, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (a VARCHAR(10) UNIQUE, b FLOAT)");
  ASSERT_THROW(ProcessQueryInternal(
                   db, "INSERT INTO test_tbl VALUES ('123', 5), ('123', 6)"),
               std::runtime_error);
}

TEST(ForeignKeyInsertFail, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY KEY, song INT, "
      "FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(db, "INSERT INTO test_tbl VALUES ('John', 'Lennon')");
  ASSERT_THROW(
      ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('Egor', 5)"),
      std::runtime_error);
}

TEST(ForeignKeyInsertGood, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY KEY, song INT, "
      "FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('Egor', 5)");
}

TEST(ForeignKeyUpdateGood, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY KEY, song INT, "
      "FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('Egor', 5)");
  ProcessQueryInternal(
      db, "UPDATE test_tbl SET first_name = 'Blah' WHERE first_name = 'John'");
}

TEST(ForeignKeyUpdateFail, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY KEY, song INT, "
      "FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('Egor', 5)");
  ASSERT_THROW(
      ProcessQueryInternal(
          db,
          "UPDATE test_tbl SET first_name = 'Blah' WHERE first_name = 'Egor'"),
      std::runtime_error);
}

TEST(ForeignKeyDeleteGood, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY KEY, song INT, "
      "FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('Egor', 5)");
  ProcessQueryInternal(db, "DELETE FROM test_tbl WHERE first_name = 'John'");
}

TEST(ForeignKeyDeleteFail, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY KEY, song INT, "
      "FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('Egor', 5)");
  ASSERT_THROW(ProcessQueryInternal(
                   db, "DELETE FROM test_tbl WHERE first_name = 'Egor'"),
               std::runtime_error);
}

TEST(UpdateGood, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(
      db, "UPDATE test_tbl SET first_name = 'Blah' WHERE first_name = 'John'");
  auto result =
      ProcessQueryInternal(db, "SELECT first_name, last_name FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Egor")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Letov")));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Blah")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Lennon")));
  ASSERT_FALSE(scan->Next());
}

TEST(UpdateForeignKeyFail, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY "
      "KEY, song INT, FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('John', 42)");
  ASSERT_THROW(
      ProcessQueryInternal(db, "UPDATE test_tbl SET first_name = 'Blah'"),
      std::runtime_error);
  auto result =
      ProcessQueryInternal(db, "SELECT first_name, last_name FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Egor")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Letov")));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("John")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Lennon")));
  ASSERT_FALSE(scan->Next());
}

TEST(DeleteForeignKeyFail, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY "
      "KEY, song INT, FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('John', 42)");
  ASSERT_THROW(ProcessQueryInternal(
                   db, "DELETE FROM test_tbl WHERE first_name = 'John'"),
               std::runtime_error);
  auto result =
      ProcessQueryInternal(db, "SELECT first_name, last_name FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Egor")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Letov")));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("John")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Lennon")));
  ASSERT_FALSE(scan->Next());
}

TEST(DeleteForeignKeyGood, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db,
      "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY "
      "KEY, song INT, FOREIGN KEY name REFERENCES test_tbl (first_name))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('John', 42)");
  ProcessQueryInternal(db, "DELETE FROM test_tbl WHERE first_name = 'Egor'");
  auto result =
      ProcessQueryInternal(db, "SELECT first_name, last_name FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("John")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Lennon")));
  ASSERT_FALSE(scan->Next());
}

TEST(DeleteAll, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "DELETE FROM test_tbl");
  auto result =
      ProcessQueryInternal(db, "SELECT first_name, last_name FROM test_tbl");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_FALSE(scan->Next());
}

TEST(SelectLeftJoin, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY "
                       "KEY, song INT)");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), "
                       "('John', 'Lennon'), ('Noname', '')");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl2 VALUES ('Egor', 5), ('Noname', 100)");
  auto result = ProcessQueryInternal(
      db,
      "SELECT first_name, last_name, song FROM test_tbl LEFT JOIN test_tbl2 t "
      "ON t.name = test_tbl.first_name");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Egor")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Letov")));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(static_cast<int>(5)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("John")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Lennon")));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(core::null_t{}));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Noname")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("")));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(static_cast<int>(100)));
  ASSERT_FALSE(scan->Next());
}

TEST(SelectInnerJoin, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl2 (name VARCHAR(255) PRIMARY "
                       "KEY, song INT)");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), "
                       "('John', 'Lennon'), ('Noname', '')");
  ProcessQueryInternal(
      db, "INSERT INTO test_tbl2 VALUES ('Egor', 5), ('Noname', 100)");
  auto result = ProcessQueryInternal(
      db,
      "SELECT first_name, last_name, song FROM test_tbl JOIN test_tbl2 t "
      "ON t.name = test_tbl.first_name");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Egor")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Letov")));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(static_cast<int>(5)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Noname")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("")));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(static_cast<int>(100)));
  ASSERT_FALSE(scan->Next());
}

TEST(SelectRightJoin, db) {
  Database db;
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl (first_name VARCHAR(255) PRIMARY "
                       "KEY, last_name VARCHAR(255))");
  ProcessQueryInternal(db,
                       "CREATE TABLE test_tbl2 (name VARCHAR(255), song INT)");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), "
                       "('John', 'Lennon'), ('Noname', '')");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl2 VALUES ('Egor', 5), ('Noname', "
                       "100), ('Blaha', 555)");
  auto result = ProcessQueryInternal(db,
                                     "SELECT first_name, last_name, name, song "
                                     "FROM test_tbl RIGHT JOIN test_tbl2 t "
                                     "ON t.name = test_tbl.first_name");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Egor")));
  ASSERT_EQ(scan->GetField("name"),
            core::FieldVariant(static_cast<std::string>("Egor")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("Letov")));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(static_cast<int>(5)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"),
            core::FieldVariant(static_cast<std::string>("Noname")));
  ASSERT_EQ(scan->GetField("name"),
            core::FieldVariant(static_cast<std::string>("Noname")));
  ASSERT_EQ(scan->GetField("last_name"),
            core::FieldVariant(static_cast<std::string>("")));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(static_cast<int>(100)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("first_name"), core::FieldVariant(core::null_t{}));
  ASSERT_EQ(scan->GetField("name"),
            core::FieldVariant(static_cast<std::string>("Blaha")));
  ASSERT_EQ(scan->GetField("last_name"), core::FieldVariant(core::null_t{}));
  ASSERT_EQ(scan->GetField("song"), core::FieldVariant(static_cast<int>(555)));
  ASSERT_FALSE(scan->Next());
}

TEST(SelectCartesianProduct, db) {
  Database db;
  ProcessQueryInternal(db, "CREATE TABLE test_tbl (a INT)");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl VALUES (1), (2), (3), (4), (5), "
                       "(6), (7), (8), (9)");
  auto result =
      ProcessQueryInternal(db,
                           "SELECT test_tbl.a, t.a FROM test_tbl, test_tbl as "
                           "t WHERE test_tbl.a * test_tbl.a = t.a");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(1)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(1)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(2)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(4)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(3)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(9)));
  ASSERT_FALSE(scan->Next());
}

TEST(AndExpr, db) {
  Database db;
  ProcessQueryInternal(db, "CREATE TABLE test_tbl (a INT)");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl VALUES (1), (2), (3), (4), (5), "
                       "(6), (7), (8), (9)");
  auto result = ProcessQueryInternal(
      db,
      "SELECT test_tbl.a, t.a FROM test_tbl, test_tbl as t WHERE test_tbl.a * "
      "test_tbl.a = t.a AND test_tbl.a != 1");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(2)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(4)));
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(3)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(9)));
  ASSERT_FALSE(scan->Next());
}

TEST(ComplexExpr, db) {
  Database db;
  ProcessQueryInternal(db, "CREATE TABLE test_tbl (a INT)");
  ProcessQueryInternal(db,
                       "INSERT INTO test_tbl VALUES (1), (2), (3), (4), (5), "
                       "(6), (7), (8), (9)");
  auto result = ProcessQueryInternal(db,
                                     R"(
      SELECT test_tbl.a, t.a
      FROM test_tbl, test_tbl AS t
      WHERE (test_tbl.a * test_tbl.a = t.a OR (test_tbl.a = 4 AND t.a = 4)) AND (test_tbl.a != 1)
    )");
  ASSERT_TRUE(result.has_value());
  auto& [scan, fields] = result.value();
  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(2)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(4)));

  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(3)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(9)));

  ASSERT_TRUE(scan->Next());
  ASSERT_EQ(scan->GetField("test_tbl.a"),
            core::FieldVariant(static_cast<int>(4)));
  ASSERT_EQ(scan->GetField("t.a"), core::FieldVariant(static_cast<int>(4)));

  ASSERT_FALSE(scan->Next());
}

}  // namespace deadfood::tests