#include "insert_into.hh"

#include <algorithm>
#include <deadfood/expr/expr_convert.hh>
#include <deadfood/expr/scan_selector/no_scan_selector.hh>
#include <deadfood/util/is_number_t.hh>
#include <functional>
#include <deadfood/expr/const_expr.hh>
#include <deadfood/expr/cmp_expr.hh>
#include <deadfood/expr/field_expr.hh>
#include <deadfood/expr/exists_expr.hh>
#include <deadfood/expr/bool_expr.hh>
#include <deadfood/scan/select_scan.hh>

#include <deadfood/exec/dml_util.hh>

namespace deadfood::exec {

void CheckIfEnoughFields(const core::Schema& schema,
                         const query::InsertQuery& query) {
  if (!query.fields.has_value()) {
    return;
  }
  std::set<std::string> query_fields{query.fields->begin(),
                                     query.fields->end()};
  for (const auto& field : schema.fields()) {
    if (!schema.MayBeNull(field) && !query_fields.contains(field)) {
      throw std::runtime_error("specify " + field + " field");
    }
  }
}

void CheckRowsSize(const query::InsertQuery& query, size_t size) {
  if (std::any_of(query.values.begin(), query.values.end(),
                  [&](const auto& row) { return row.size() != size; })) {
    throw std::runtime_error("got invalid rows");
  }
}

std::vector<std::vector<core::FieldVariant>> RetrieveValues(
    const core::Schema& schema, const std::vector<std::string>& fields,
    const query::InsertQuery& query) {
  expr::ExprTreeConverter converter{std::make_unique<expr::NoScanSelector>()};
  std::vector<std::vector<core::FieldVariant>> actual_values;

  for (const auto& row : query.values) {
    std::vector<core::FieldVariant> actual_values_row;
    for (size_t i = 0; i < row.size(); ++i) {
      auto e = converter.ConvertExprTreeToIExpr(row[i]);
      auto val = e->Eval();
      util::ValidateType(schema.MayBeNull(fields[i]),
                         schema.field_info(fields[i]), val);

      const auto field_type = schema.field_info(fields[i]).type();
      actual_values_row.emplace_back(
          util::NormalizeFieldVariant(field_type, val));
    }
    actual_values.emplace_back(std::move(actual_values_row));
  }
  return actual_values;
}

void ExecuteInsertQuery(Database& db, const query::InsertQuery& query) {
  if (!db.Exists(query.table_name)) {
    throw std::runtime_error("table does not exist");
  }
  const auto& schema = db.schemas().at(query.table_name);
  CheckIfEnoughFields(schema, query);

  std::vector<std::string> fields{query.fields.value_or(schema.fields())};
  CheckRowsSize(query, fields.size());

  auto actual_values = RetrieveValues(schema, fields, query);

  std::map<std::string, std::set<core::FieldVariant>> already_in_table;

  for (const auto& row : actual_values) {
    for (size_t i = 0; i < row.size(); ++i) {
      if (schema.IsUnique(fields[i])) {
        util::CheckUniquenessConstraint(db, query.table_name, fields[i],
                                        row[i]);
        if (already_in_table[fields[i]].contains(row[i])) {
          throw std::runtime_error("unique key constraint violated");
        }
        already_in_table[fields[i]].emplace(row[i]);
      }
      util::CheckForeignKeyConstraintInInsertQuery(db, query.table_name,
                                                   fields[i], row[i]);
    }
  }
  already_in_table.clear();

  auto scan = db.GetTableScan(query.table_name);
  scan->BeforeFirst();
  for (const auto& row : actual_values) {
    scan->Insert();
    for (size_t i = 0; i < row.size(); ++i) {
      scan->SetField(fields[i], row[i]);
    }
  }
}

}  // namespace deadfood::exec