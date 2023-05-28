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

struct ValidateTypeVisitor {
  bool may_be_null;
  const core::Field& field_info;

  void operator()(const auto& value) {
    using T = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<T, core::null_t>) {
      if (!may_be_null) {
        throw std::runtime_error("passed null to non-null field");
      }
    } else if constexpr (deadfood::util::IsNumberT<T>::value) {
      if (field_info.type() == core::Field::FieldType::Varchar) {
        throw std::runtime_error("cannot convert number into string");
      }
    } else if constexpr (std::is_same_v<T, std::string>) {
      if (field_info.type() != core::Field::FieldType::Varchar) {
        throw std::runtime_error("cannot convert string into non-string");
      }
      if (field_info.size() < value.size()) {
        throw std::runtime_error("the string is too large");
      }
    }
  }
};

std::vector<std::vector<core::FieldVariant>> RetrieveValues(
    const core::Schema& schema, const std::vector<std::string>& fields,
    const query::InsertQuery& query) {
  expr::ExprTreeConverter converter{
      std::make_unique<expr::NoScanSelector>()};
  std::vector<std::vector<core::FieldVariant>> actual_values;

  for (const auto& row : query.values) {
    std::vector<core::FieldVariant> actual_values_row;
    for (size_t i = 0; i < row.size(); ++i) {
      auto e = converter.ConvertExprTreeToIExpr(row[i]);
      auto val = e->Eval();

      std::visit(ValidateTypeVisitor{schema.MayBeNull(fields[i]),
                                     schema.field_info(fields[i])},
                 val);
      auto norm_val = std::visit(
          [&](auto&& arg) -> core::FieldVariant {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (deadfood::util::IsNumberT<T>::value) {
              const auto info = schema.field_info(fields[i]);
              switch (info.type()) {
                case core::Field::FieldType::Bool:
                  return static_cast<bool>(arg);
                case core::Field::FieldType::Int:
                  return static_cast<int>(arg);
                case core::Field::FieldType::Float:
                  return static_cast<float>(arg);
                case core::Field::FieldType::Double:
                  return static_cast<double>(arg);
                case core::Field::FieldType::Varchar:
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
}

void CheckUniquenessConstraint(Database& db, const std::string& table_name,
                               const std::string& field_name,
                               const core::FieldVariant& value) {
  auto scan = db.GetTableScan(table_name);

  std::unique_ptr<expr::IExpr> predicate = std::make_unique<expr::CmpExpr>(
      expr::CmpOp::Eq, std::make_unique<expr::ConstExpr>(value),
      std::make_unique<expr::FieldExpr>(scan.get(), field_name));

  expr::ExistsExpr exists(std::make_unique<scan::SelectScan>(
      std::move(scan), expr::BoolExpr(std::move(predicate))));
  if (std::get<bool>(exists.Eval())) {
    throw std::runtime_error("unique constraint violated");
  }
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

  auto scan = db.GetTableScan(query.table_name);
  scan->BeforeFirst();
  for (const auto& row : actual_values) {
    scan->Insert();
    for (size_t i = 0; i < row.size(); ++i) {
      if (schema.IsUnique(fields[i])) {
        CheckUniquenessConstraint(db, query.table_name, fields[i], row[i]);
      }
      scan->SetField(fields[i], row[i]);
    }
  }
}

}  // namespace deadfood::exec