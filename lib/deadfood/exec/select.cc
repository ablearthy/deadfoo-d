#include "select.hh"

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

#include <deadfood/util/str.hh>

namespace deadfood::exec {

void CheckIfFieldExists(const Database& db,
                        const std::map<std::string, size_t>& variables,
                        const std::map<std::string, std::string>& aliases,
                        const std::string& value) {
  if (auto s = util::SplitOnDot(value)) {
    const auto& [table_name, field_name] = s.value();
    if (db.table_names().contains(table_name)) {
      if (!db.schemas().at(table_name).Exists(field_name)) {
        throw std::runtime_error("variable `" + value + "` is not in scope");
      }
    } else if (aliases.contains(table_name)) {
      if (!db.schemas().at(aliases.at(table_name)).Exists(field_name)) {
        throw std::runtime_error("variable `" + value + "` is not in scope");
      }
    } else {
      throw std::runtime_error("unknown table name");
    }
  } else {
    if (!variables.contains(value)) {
      throw std::runtime_error("variable `" + value + "` is not in scope");
    }
    if (variables.at(value) > 1) {
      throw std::runtime_error("variable `" + value + "` is ambiguous");
    }
  }
}

struct Y {
  const Database& db;
  const std::map<std::string, size_t>& variables;
  const std::map<std::string, std::string>& aliases;

  void operator()(const expr::ExprTree& expr) const {
    for (const auto& v : expr.factors) {
      std::visit(*this, v.factor);
    }
  }

  void operator()(const expr::ExprId& expr) const {
    CheckIfFieldExists(db, variables, aliases, expr.id);
  }

  void operator()(const expr::Constant& expr) const {}
};

struct X {
  Database& db;
  std::map<std::string, size_t> variables;
  std::map<std::string, std::string> aliases;

  void operator()(const query::SelectQuery& query) {
    for (const auto& source : query.sources) {
      std::visit(*this, source);
    }
    auto select_from_variables = variables;
    for (const auto& source : query.sources) {
      if (auto* r = std::get_if<query::FromTable>(&source)) {
        for (const auto& field_name : db.schemas().at(r->table_name).fields()) {
          ++select_from_variables[field_name];
        }
      }
    }

    for (const auto& join : query.joins) {
      operator()(query::FromTable{.table_name = join.table_name,
                                  .renamed = join.alias});
      std::visit(Y{db, variables, aliases}, join.predicate.factor);

      for (const auto& field_name : db.schemas().at(join.table_name).fields()) {
        ++select_from_variables[field_name];
      }
    }

    for (const auto& selector : query.selectors) {
      std::visit(
          [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::string>) {
              CheckIfFieldExists(db, select_from_variables, aliases, arg);
            } else if constexpr (std::is_same_v<T, query::FieldSelector>) {
              if (select_from_variables.contains(arg.field_name)) {
                throw std::runtime_error("variable `" + arg.field_name +
                                         "` introduces ambiguity");
              }
              std::visit(Y{db, select_from_variables, aliases},
                         arg.expr.factor);
            }
          },
          selector);
    }

    for (const auto& selector : query.selectors) {
      std::visit(
          [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, query::FieldSelector>) {
              ++variables[arg.field_name];
            } else if constexpr (std::is_same_v<T, std::string>) {
              ++variables[arg];
            } else if constexpr (std::is_same_v<T, query::SelectAllSelector>) {
              for (const auto& source : query.sources) {
                if (auto* r = std::get_if<query::FromTable>(&source)) {
                  for (const auto& field_name :
                       db.schemas().at(r->table_name).fields()) {
                    if (!variables.contains(field_name)) {
                      ++variables[field_name];
                    }
                  }
                }
              }
            }
          },
          selector);
    }

    if (query.predicate.has_value()) {
      std::visit(Y{db, variables, aliases}, query.predicate.value().factor);
    }
  }

  void operator()(const query::FromTable& query) {
    if (!db.Exists(query.table_name)) {
      throw std::runtime_error("table `" + query.table_name +
                               "` does not exist");
    }
    if (aliases.contains(query.table_name)) {
      throw std::runtime_error("alias with name `" + query.table_name +
                               "` already exists (collision with table name)");
    }
    if (query.renamed.has_value()) {
      if (db.Exists(query.renamed.value())) {
        throw std::runtime_error("alias `" + query.renamed.value() +
                                 "` introduces ambiguity");
      }
      aliases.emplace(query.renamed.value(), query.table_name);
    }
  }
};

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
      std::unique_ptr<scan::IScan> tmp = std::make_unique<scan::ProductScan>(std::move(scan),
          GetScanFromSource(db, query.sources[i]));
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
              ret.emplace_back(sel);
            } else if constexpr (std::is_same_v<T, query::FieldSelector>) {
              ret.emplace_back(sel.field_name);
            }
          },
          selector);
    }
    return ret;
  }
};

std::pair<std::unique_ptr<scan::IScan>, std::vector<std::string>>
ExecuteSelectQuery(Database& db, const query::SelectQuery& query) {
  X{.db = db}(query);
  auto scan = GetScanFromSelectQuery(db, query);

  ObtainAllFieldsVisitor vis{db};
  std::vector<std::string> fields = vis(query);
  scan->BeforeFirst();
  return {std::move(scan), fields};
}

}  // namespace deadfood::exec