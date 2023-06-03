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
  ProcessQueryInternal(db, "INSERT INTO test_tbl VALUES ('Egor', 'Letov'), ('John', 'Lennon')");
  ProcessQueryInternal(db, "INSERT INTO test_tbl2 VALUES ('Egor', 5)");
}

}  // namespace deadfood::tests