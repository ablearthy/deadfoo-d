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
#include <deadfood/expr/expr_convert.hh>
#include <deadfood/expr/insert_into_get_table_scan.hh>

#include <deadfood/util/is_number_t.hh>
#include <deadfood/expr/const_expr.hh>
#include <deadfood/parse/select_parser.hh>
#include <deadfood/exec/select/find_table_by_field.hh>
#include <deadfood/scan/extend_scan.hh>

using namespace deadfood::core;
using namespace deadfood::storage;
using namespace deadfood::scan;
using namespace deadfood::expr;
using namespace deadfood::lex;
using namespace deadfood::parse;
using namespace deadfood::query;
using namespace deadfood::exec;
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
    schema.AddField(field_name, field, q.MayBeNull(field_name),
                    q.IsUnique(field_name));
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

void ExecuteInsertQuery(Database& db, const InsertQuery& query) {
  if (!db.Exists(query.table_name)) {
    throw std::runtime_error("table does not exist");
  }
  if (query.fields.has_value()) {
    // check not null
    const auto& schema = db.schemas().at(query.table_name);
    std::set<std::string> query_fields{query.fields->begin(),
                                       query.fields->end()};
    for (const auto& field : schema.fields()) {
      if (!schema.MayBeNull(field) && !query_fields.contains(field)) {
        throw std::runtime_error("specify " + field + " field");
      }
    }
  }

  const auto schema = db.schemas().at(query.table_name);
  std::vector<std::string> fields;
  if (query.fields.has_value()) {
    fields = query.fields.value();
  } else {
    fields = schema.fields();
  }
  for (const auto& row : query.values) {
    if (row.size() != fields.size()) {
      throw std::runtime_error("got invalid rows");
    }
  }

  ExprTreeConverter converter{std::make_unique<InsertIntoGetTableScan>()};
  std::vector<std::vector<core::FieldVariant>> actual_values;
  for (const auto& row : query.values) {
    if (row.size() != fields.size()) {
      throw std::runtime_error("got invalid rows");
    }
    std::vector<core::FieldVariant> actual_values_row;
    for (size_t i = 0; i < row.size(); ++i) {
      auto e = converter.ConvertExprTreeToIExpr(row[i]);
      auto val = e->Eval();
      std::visit(
          [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, core::null_t>) {
              if (!schema.MayBeNull(fields[i])) {
                throw std::runtime_error("passed null to non-null field");
              }
            } else if constexpr (deadfood::util::IsNumberT<T>::value) {
              const auto info = schema.field_info(fields[i]);
              if (info.type() == core::Field::FieldType::Varchar) {
                throw std::runtime_error("cannot convert number into string");
              }
            } else if constexpr (std::is_same_v<T, std::string>) {
              const auto info = schema.field_info(fields[i]);
              if (info.type() != core::Field::FieldType::Varchar) {
                throw std::runtime_error(
                    "cannot convert string into non-string");
              }
              if (info.size() < arg.size()) {
                throw std::runtime_error("the string is too large");
              }
            }
          },
          val);
      auto norm_val = std::visit(
          [&](auto&& arg) -> core::FieldVariant {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (deadfood::util::IsNumberT<T>::value) {
              const auto info = schema.field_info(fields[i]);
              switch (info.type()) {
                case Field::FieldType::Bool:
                  return static_cast<bool>(arg);
                case Field::FieldType::Int:
                  return static_cast<int>(arg);
                case Field::FieldType::Float:
                  return static_cast<float>(arg);
                case Field::FieldType::Double:
                  return static_cast<double>(arg);
                case Field::FieldType::Varchar:
                  throw std::runtime_error("cannot convert number into string");
              }
            }
            return arg;
          },
          val);

      actual_values_row.emplace_back(std::move(norm_val));
    }
    actual_values.emplace_back(std::move(actual_values_row));
  }
  auto scan = db.GetTableScan(query.table_name);
  scan->BeforeFirst();
  for (const auto& row : actual_values) {
    scan->Insert();
    for (size_t i = 0; i < row.size(); ++i) {
      if (schema.IsUnique(fields[i])) {
        auto unique_scan = db.GetTableScan(query.table_name);
        std::unique_ptr<IExpr> predicate = std::make_unique<CmpExpr>(
            CmpOp::Eq, std::make_unique<ConstExpr>(row[i]),
            std::make_unique<FieldExpr>(unique_scan.get(), fields[i]));
        ExistsExpr exists(std::make_unique<SelectScan>(
            std::move(unique_scan), BoolExpr(std::move(predicate))));
        if (std::get<bool>(exists.Eval())) {
          throw std::runtime_error("unique constraint violated");
        }
      }
      scan->SetField(fields[i], row[i]);
    }
  }
}

std::unique_ptr<IScan> GetScanFromSelectQuery(Database& db,
                                              const query::SelectQuery& query);

std::unique_ptr<IScan> GetScanFromSource(Database& db,
                                         const query::SelectFrom& from) {
  return std::visit(
      [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, SelectQuery>) {
          return GetScanFromSelectQuery(db, arg);
        } else if constexpr (std::is_same_v<T, FromTable>) {
          if (arg.renamed.has_value()) {
            return db.GetTableScan(arg.table_name, arg.renamed.value());
          } else {
            return db.GetTableScan(arg.table_name);
          }
        }
      },
      from);
}

class SelectFromGetTableScan : public GetTableScan {
 public:
  SelectFromGetTableScan(IScan* internal) : internal_{internal} {}

  IScan* GetScan(const std::string& field_name) override { return internal_; }

 private:
  IScan* internal_;
};

std::unique_ptr<IScan> GetScanFromSelectQuery(Database& db,
                                              const query::SelectQuery& query) {
  auto scan = GetScanFromSource(db, query.sources[0]);
  for (size_t i = 1; i < query.sources.size(); ++i) {
    std::unique_ptr<IScan> tmp = std::make_unique<ProductScan>(
        GetScanFromSource(db, query.sources[i]), std::move(scan));
    scan = std::move(tmp);
  }

  for (const auto& join : query.joins) {
    // TODO: left & right join support
    std::unique_ptr<IScan> tmp = std::make_unique<ProductScan>(
        db.GetTableScan(join.table_name, join.alias), std::move(scan));

    std::string lhs_field_name =
        join.table_name_lhs + "." + join.field_name_lhs;
    std::string rhs_field_name =
        join.table_name_rhs + "." + join.field_name_rhs;
    scan = std::make_unique<SelectScan>(
        std::move(tmp),
        expr::BoolExpr(std::make_unique<CmpExpr>(
            CmpOp::Eq, std::make_unique<FieldExpr>(tmp.get(), lhs_field_name),
            std::make_unique<FieldExpr>(tmp.get(), rhs_field_name))));
  }

  for (const auto& selector : query.selectors) {
    if (auto s = std::get_if<FieldSelector>(&selector)) {
      ExprTreeConverter converter{
          std::make_unique<SelectFromGetTableScan>(scan.get())};

      scan = std::make_unique<ExtendScan>(
          std::move(scan), converter.ConvertExprTreeToIExpr(s->expr),
          s->field_name);
    }
  }

  if (query.predicate.has_value()) {
    ExprTreeConverter converter{
        std::make_unique<SelectFromGetTableScan>(scan.get())};
    scan = std::make_unique<SelectScan>(
        std::move(scan), expr::BoolExpr(converter.ConvertExprTreeToIExpr(
                             query.predicate.value())));
  }

  return scan;
}

struct ObtainAllFieldsVisitor {
  Database& db;

  std::vector<std::string> operator()(const query::FromTable& from_table) {
    std::vector<std::string> ret;
    const auto& schema = db.schemas().at(from_table.table_name);
    std::string table_name = from_table.renamed.has_value()
                                 ? from_table.renamed.value()
                                 : from_table.table_name;
    table_name += '.';
    for (const auto& field_name : schema.fields()) {
      ret.emplace_back(table_name + field_name);
    }
    return ret;
  }

  std::vector<std::string> operator()(const query::SelectQuery& query) {
    std::vector<std::string> ret;
    for (const auto& selector : query.selectors) {
      std::visit(
          [&](auto&& sel) {
            using T = std::decay_t<decltype(sel)>;
            if constexpr (std::is_same_v<T, SelectAllSelector>) {
              for (const auto& source : query.sources) {
                const auto inner_fields = std::visit(*this, source);
                ret.insert(ret.end(), inner_fields.begin(), inner_fields.end());
              }
              for (const auto& join : query.joins) {
                const auto inner_fields = operator()(FromTable{
                    .table_name = join.table_name, .renamed = join.alias});
                ret.insert(ret.end(), inner_fields.begin(), inner_fields.end());
              }
            } else if constexpr (std::is_same_v<T, std::string>) {
              if (const auto c = FindTableByField(db.schemas(), query, sel)) {
                ret.emplace_back(c->full_field_name);
              } else {
                ret.emplace_back(sel);
              }
            } else if constexpr (std::is_same_v<T, FieldSelector>) {
              if (FindTableByField(db.schemas(), query, sel.field_name)
                      .has_value()) {
                throw std::runtime_error("ambiguous source for `" +
                                         sel.field_name + "` field");
              }
              ret.emplace_back(sel.field_name);
            }
          },
          selector);
    }
    return ret;
  }
};

void ExecuteSelectQuery(Database& db, const query::SelectQuery& query) {
  auto scan = GetScanFromSelectQuery(db, query);

  ObtainAllFieldsVisitor vis{db};
  std::vector<std::string> fields = vis(query);

  scan->BeforeFirst();
  for (size_t i = 0; i < fields.size(); ++i) {
    std::cout << fields[i];
    if (i != fields.size() - 1) {
      std::cout << '|';
    }
  }
  std::cout << '\n';
  while (scan->Next()) {
    for (size_t i = 0; i < fields.size(); ++i) {
      const auto value = scan->GetField(fields[i]);
      std::visit(
          [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, core::null_t>) {
              std::cout << "NULL";
            } else if constexpr (std::is_same_v<T, std::string>) {
              std::cout << '\'' << arg << '\'';  // TODO: handle \n...
            } else {
              std::cout << arg;
            }
          },
          value);
      if (i != fields.size() - 1) {
        std::cout << '|';
      }
    }
    std::cout << '\n';
  }
}

void ProcessQueryInternal(Database& db, const std::vector<Token>& tokens) {
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
    const auto q = ParseSelectQuery(tokens);
    ExecuteSelectQuery(db, q);
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
  //  auto db = Load("/tmp/f");
  //  for (const auto& tbl : db.table_names()) {
  //    std::cout << tbl << '\n';
  //  }
  char* query_buf;
  while ((query_buf = readline("> ")) != nullptr) {
    std::string query{query_buf};
    add_history(query_buf);

    free(query_buf);
    ProcessQuery(db, query);
  }
  //      Dump(db, "/tmp/f");
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
