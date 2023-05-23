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
#include <deadfood/database.hh>

#include <readline/readline.h>
#include <readline/history.h>

using namespace deadfood::core;
using namespace deadfood::storage;
using namespace deadfood::scan;
using namespace deadfood::expr;
using namespace deadfood::lex;
using namespace deadfood::parse;
using namespace deadfood;

void ProcessQuery(Database& db, const std::string& query) {
  try {
    const auto tokens = Lex(query);
  } catch (const std::runtime_error& err) {
    std::cout << "[error (lex)] " << err.what() << '\n';
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
