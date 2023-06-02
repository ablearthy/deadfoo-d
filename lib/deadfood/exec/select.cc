#include "select.hh"

#include <iostream>

#include <deadfood/scan/iscan.hh>
#include <deadfood/scan/product_scan.hh>
#include <deadfood/expr/bool_expr.hh>
#include <deadfood/scan/select_scan.hh>
#include <deadfood/expr/cmp_expr.hh>
#include <deadfood/expr/field_expr.hh>
#include <deadfood/scan/left_join_scan.hh>
#include <deadfood/expr/expr_convert.hh>
#include <deadfood/expr/scan_selector/simple_scan_selector.hh>
#include <deadfood/scan/extend_scan.hh>
#include <deadfood/exec/select/find_table_by_field.hh>
#include <deadfood/expr/const_expr.hh>

namespace deadfood::exec {

std::unique_ptr<scan::IScan> GetScanFromSelectQuery(
    Database& db, const query::SelectQuery& query);

std::unique_ptr<scan::IScan> GetScanFromSource(Database& db,
                                               const query::SelectFrom& from) {
  return std::visit(
      [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, query::SelectQuery>) {
          return GetScanFromSelectQuery(db, arg);
        } else if constexpr (std::is_same_v<T, query::FromTable>) {
          if (!db.table_names().contains(arg.table_name)) {
            throw std::runtime_error("unknown table `" + arg.table_name + "`");
          }
          if (arg.renamed.has_value()) {
            return db.GetTableScan(arg.table_name, arg.renamed.value());
          } else {
            return db.GetTableScan(arg.table_name);
          }
        }
      },
      from);
}

std::unique_ptr<scan::IScan> GetScanFromSelectQuery(
    Database& db, const query::SelectQuery& query) {
  std::unique_ptr<scan::IScan> scan;
  if (!query.sources.empty()) {
    scan = GetScanFromSource(db, query.sources[0]);
    for (size_t i = 1; i < query.sources.size(); ++i) {
      std::unique_ptr<scan::IScan> tmp = std::make_unique<scan::ProductScan>(
          GetScanFromSource(db, query.sources[i]), std::move(scan));
      scan = std::move(tmp);
    }
  }

  for (const auto& join : query.joins) {
    if (join.type == query::JoinType::Inner) {
      std::unique_ptr<scan::IScan> tmp = std::make_unique<scan::ProductScan>(
          db.GetTableScan(join.table_name, join.alias), std::move(scan));
      expr::ExprTreeConverter converter{
          std::make_unique<expr::SimpleScanSelector>(tmp.get())};
      scan = std::make_unique<scan::SelectScan>(
          std::move(tmp),
          expr::BoolExpr(converter.ConvertExprTreeToIExpr(join.predicate)));
    } else if (join.type == query::JoinType::Left) {
      std::unique_ptr<scan::LeftJoinScan> tmp =
          std::make_unique<scan::LeftJoinScan>(
              std::move(scan), db.GetTableScan(join.table_name, join.alias),
              std::make_unique<expr::ConstExpr>(true));
      expr::ExprTreeConverter converter{
          std::make_unique<expr::SimpleScanSelector>(tmp.get())};
      tmp->set_predicate(converter.ConvertExprTreeToIExpr(join.predicate));
      scan = std::move(tmp);
    } else if (join.type == query::JoinType::Right) {
      std::unique_ptr<scan::LeftJoinScan> tmp =
          std::make_unique<scan::LeftJoinScan>(
              db.GetTableScan(join.table_name, join.alias), std::move(scan),
              std::make_unique<expr::ConstExpr>(true));
      expr::ExprTreeConverter converter{
          std::make_unique<expr::SimpleScanSelector>(tmp.get())};
      tmp->set_predicate(converter.ConvertExprTreeToIExpr(join.predicate));
      scan = std::move(tmp);
    } else {
      throw std::runtime_error("unhandled join type");
    }
  }

  for (const auto& selector : query.selectors) {
    if (auto s = std::get_if<query::FieldSelector>(&selector)) {
      expr::ExprTreeConverter converter{
          std::make_unique<expr::SimpleScanSelector>(scan.get())};
      scan = std::make_unique<scan::ExtendScan>(
          std::move(scan), converter.ConvertExprTreeToIExpr(s->expr),
          s->field_name);
    }
  }

  if (query.predicate.has_value()) {
    expr::ExprTreeConverter converter{
        std::make_unique<expr::SimpleScanSelector>(scan.get())};
    scan = std::make_unique<scan::SelectScan>(
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
            if constexpr (std::is_same_v<T, query::SelectAllSelector>) {
              for (const auto& source : query.sources) {
                const auto inner_fields = std::visit(*this, source);
                ret.insert(ret.end(), inner_fields.begin(), inner_fields.end());
              }
              for (const auto& join : query.joins) {
                const auto inner_fields = operator()(query::FromTable{
                    .table_name = join.table_name, .renamed = join.alias});
                ret.insert(ret.end(), inner_fields.begin(), inner_fields.end());
              }
            } else if constexpr (std::is_same_v<T, std::string>) {
              if (const auto c = FindTableByField(db.schemas(), query, sel)) {
                ret.emplace_back(c->full_field_name);
              } else {
                ret.emplace_back(sel);
              }
            } else if constexpr (std::is_same_v<T, query::FieldSelector>) {
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
  while (true) {
    try {
      if (!scan->Next()) {
        break;
      }
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
    } catch (const std::out_of_range& ex) {
      throw std::runtime_error("failed to execute query");
    }
  }
}

}  // namespace deadfood::exec