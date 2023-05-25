#include <iostream>
#include <memory>

#include <deadfood/storage/db_storage.hh>
#include <deadfood/storage/table_storage.hh>
#include <deadfood/scan/table_scan.hh>
#include <deadfood/core/schema.hh>
#include <deadfood/scan/product_scan.hh>
#include <deadfood/expr/iexpr.hh>
#include <deadfood/expr/cmp_expr.hh>
#include <deadfood/expr/field_expr.hh>
#include <deadfood/expr/bool_expr.hh>
#include <deadfood/scan/select_scan.hh>
#include <deadfood/lex/lex.hh>
#include <deadfood/parse/create_table_parser.hh>
#include <deadfood/parse/drop_table_parser.hh>
#include <deadfood/parse/insert_parser.hh>
#include <deadfood/parse/parser_error.hh>
#include <deadfood/database.hh>
#include <deadfood/parse/expr_tree_parser.hh>

#include <readline/readline.h>
#include <readline/history.h>
#include <deadfood/expr/exists_expr.hh>

using namespace deadfood::core;
using namespace deadfood::storage;
using namespace deadfood::scan;
using namespace deadfood::expr;
using namespace deadfood::lex;
using namespace deadfood::parse;
using namespace deadfood::query;
using namespace deadfood;

void ExecuteCreateTableQuery(
    Database& db,
    const std::pair<query::CreateTableQuery, std::vector<core::Constraint>>&
        query) {
  const auto& [q, constraints] = query;
  if (db.Exists(q.table_name())) {
    throw std::runtime_error("table already exists");
  }
  Schema schema;
  for (const auto& field_name : q.field_names()) {
    const auto field = q.GetField(field_name);
    schema.AddField(field_name, field);
  }
  db.AddTable(q.table_name(), schema);
  for (const auto& c : constraints) {
    db.constraints().emplace_back(c);
  }
}

void ExecuteDropTableQuery(Database& db, const std::string& table_name) {
  if (!db.Exists(table_name)) {
    throw std::runtime_error("table does not exist");
  }
  for (const auto& constr : db.constraints()) {
    if (!std::holds_alternative<ReferencesConstraint>(constr)) {
      continue;
    }
    const auto ref_constr = std::get<ReferencesConstraint>(constr);
    if (ref_constr.master_table == table_name &&
        ref_constr.on_delete == ReferencesConstraint::OnAction::NoAction) {
      auto scan_slave = db.GetTableScan(ref_constr.slave_table);
      auto scan_master = db.GetTableScan(ref_constr.master_table);

      // EXISTS(SELECT 1 FROM master_table WHERE EXISTS(SELECT 1 FROM
      // slave_table WHERE slave_table.slave_field = master_table.master_field))

      std::unique_ptr<IExpr> cmp_fields = std::make_unique<CmpExpr>(
          CmpOp::Eq,
          std::make_unique<FieldExpr>(scan_slave.get(), ref_constr.slave_field),
          std::make_unique<FieldExpr>(scan_master.get(),
                                      ref_constr.master_field));
      std::unique_ptr<IScan> inner_select = std::make_unique<SelectScan>(
          std::move(scan_slave), expr::BoolExpr(std::move(cmp_fields)));
      std::unique_ptr<IExpr> expr =
          std::make_unique<ExistsExpr>(std::move(inner_select));
      std::unique_ptr<IScan> select = std::make_unique<SelectScan>(
          std::move(scan_master), expr::BoolExpr(std::move(expr)));

      ExistsExpr exists(std::move(select));
      if (std::get<bool>(exists.Eval())) {
        throw std::runtime_error("foreign key violated");
      }
    }
  }
  db.RemoveTable(table_name);
}

void ExecuteInsertQuery(Database& db, const InsertQuery& query) {}

void ProcessQueryInternal(Database& db, const std::vector<Token> tokens) {
  if (IsKeyword(tokens[0], Keyword::Create)) {  // create table query
    const auto q = ParseCreateTableQuery(tokens);
    ExecuteCreateTableQuery(db, q);
  } else if (IsKeyword(tokens[0], Keyword::Drop)) {  // drop table query
    const auto q = ParseDropTableQuery(tokens);
    ExecuteDropTableQuery(db, q);
  } else if (IsKeyword(tokens[0], Keyword::Update)) {  // update query

  } else if (IsKeyword(tokens[0], Keyword::Delete)) {  // delete query

  } else if (IsKeyword(tokens[0], Keyword::Insert)) {  // insert query
    const auto q = ParseInsertQuery(tokens);
    ExecuteInsertQuery(db, q);
  } else if (IsKeyword(tokens[0], Keyword::Select)) {  // select query

  } else {
    std::cout << "unknown query\n";
    return;
  }
}
void ProcessQuery(Database& db, const std::string& query) {
  std::vector<Token> tokens;
  try {
    tokens = Lex(query);
  } catch (const std::runtime_error& err) {
    std::cout << "[error (lex)] " << err.what() << '\n';
    return;
  }
  if (tokens.empty()) {
    std::cout << "expected some input\n";
    return;
  }
  try {
    ProcessQueryInternal(db, tokens);
  } catch (const ParserError& e) {
    std::cout << "[error (parse)] " << e.what() << '\n';
    return;
  } catch (const std::runtime_error& err) {
    std::cout << "[error (runtime)] " << err.what() << '\n';
    return;
  }
}

int main() {
  Database db;

  char* query_buf;
  while ((query_buf = readline("> ")) != nullptr) {
    std::string query{query_buf};
    add_history(query_buf);

    free(query_buf);
    ProcessQuery(db, query);
  }
}

//  TableStorage storage1, storage2;
//
//  Schema schema1, schema2;
//
//  schema1.AddField("a", field::kBoolField);
//  schema1.AddField("b", field::kIntField);
//  schema1.AddField("c", field::kFloatField);
//
//  schema2.AddField("d", field::kIntField);
//
//  std::unique_ptr<IScan> scan1 = std::make_unique<TableScan>(storage1,
//  schema1); std::unique_ptr<IScan> scan2 =
//  std::make_unique<TableScan>(storage2, schema2);
//
//  scan1->BeforeFirst();
//  scan2->BeforeFirst();
//  float x = 42.0;
//  bool b = true;
//  for (int i = 2; i < 10; ++i) {
//    scan1->Insert();
//    scan1->SetField("a", b);
//    scan1->SetField("b", i);
//    scan1->SetField("c", x);
//
//    scan2->Insert();
//    scan2->SetField("d", static_cast<int>(x));
//    b = !b;
//    x *= 5;
//  }
//
//  //  scan2->BeforeFirst();
//  //  while (scan2->Next()) {
//  //    std::cout << std::get<int>(scan2->GetField("d")) << '\n';
//  //  }
//
//  std::unique_ptr<IExpr> e1{std::make_unique<FieldExpr>(scan1.get(), "c")};
//  std::unique_ptr<IExpr> e2{std::make_unique<FieldExpr>(scan2.get(), "d")};
//
//  std::unique_ptr<IExpr> expr{
//      std::make_unique<CmpExpr>(CmpOp::Eq, std::move(e1), std::move(e2))};
//
//  BoolExpr predicate{std::move(expr)};
//
//  std::unique_ptr<IScan> ps{
//      std::make_unique<ProductScan>(std::move(scan1), std::move(scan2))};
//
//  std::unique_ptr<IScan> ss{std::make_unique<SelectScan>(std::move(ps),
//  std::move(predicate))}; ss->BeforeFirst(); while (ss->Next()) {
//    std::cout << std::get<bool>(ss->GetField("a")) << ' '
//              << std::get<int>(ss->GetField("b")) << ' '
//              << std::get<float>(ss->GetField("c")) << ' '
//              << std::get<int>(ss->GetField("d")) << '\n';
//  }
