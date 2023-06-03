#include "update.hh"

#include <deadfood/expr/expr_convert.hh>
#include <deadfood/expr/scan_selector/simple_scan_selector.hh>
#include <deadfood/scan/select_scan.hh>

#include <deadfood/exec/dml_util.hh>

namespace deadfood::exec {

void ValidateFieldNames(const core::Schema& schema,
                        const query::UpdateQuery& query) {
  for (const auto& [field_name, _] : query.sets) {
    if (!schema.Exists(field_name)) {
      throw std::runtime_error("field `" + field_name + "` does not exist");
    }
  }
}

void ExecuteUpdateQuery(Database& db, const query::UpdateQuery& query) {
  if (!db.Exists(query.table_name)) {
    throw std::runtime_error("table does not exist");
  }
  const auto& schema = db.schemas().at(query.table_name);
  ValidateFieldNames(schema, query);

  auto scan = db.GetTableScan(query.table_name);
  if (query.predicate.has_value()) {
    expr::ExprTreeConverter conv{
        std::make_unique<expr::SimpleScanSelector>(scan.get())};
    scan = std::make_unique<scan::SelectScan>(
        std::move(scan),
        expr::BoolExpr(conv.ConvertExprTreeToIExpr(query.predicate.value())));
  }
  expr::ExprTreeConverter conv{
      std::make_unique<expr::SimpleScanSelector>(scan.get())};
  std::map<std::string, std::unique_ptr<expr::IExpr>> expression_map;

  for (const auto& [field_name, expr_tree] : query.sets) {
    expression_map.emplace(field_name, conv.ConvertExprTreeToIExpr(expr_tree));
  }

  std::vector<std::map<std::string, core::FieldVariant>> updated_values;
  while (scan->Next()) {
    std::map<std::string, core::FieldVariant> row;
    for (const auto& [field_name, expr] : expression_map) {
      auto value = expr->Eval();
      const auto field_info = schema.field_info(field_name);
      util::ValidateType(schema.MayBeNull(field_name), field_info, value);
      if (schema.IsUnique(field_name)) {
        util::CheckUniquenessConstraint(db, query.table_name, field_name,
                                        value);
      }
      util::CheckForeignKeyConstraint(db, query.table_name, field_name,
                                      scan->GetField(field_name),
                                      util::Action::Update);
      row.emplace(field_name,
                  util::NormalizeFieldVariant(field_info.type(), value));
    }
    updated_values.emplace_back(std::move(row));
  }
  scan->BeforeFirst();
  auto it = updated_values.begin();
  while (scan->Next()) {
    for (const auto& [field_name, value] : *it) {
      scan->SetField(field_name, value);
    }
    ++it;
  }
}

}  // namespace deadfood::exec