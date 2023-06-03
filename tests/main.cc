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

}  // namespace deadfood::tests