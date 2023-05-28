#include "dml_util.hh"

#include <deadfood/util/is_number_t.hh>

#include <deadfood/expr/iexpr.hh>
#include <deadfood/expr/exists_expr.hh>
#include <deadfood/expr/cmp_expr.hh>
#include <deadfood/expr/field_expr.hh>
#include <deadfood/expr/const_expr.hh>

#include <deadfood/scan/select_scan.hh>

#include <deadfood/exec/dql_util.hh>

namespace deadfood::exec::util {

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

void ValidateType(bool may_be_null, const core::Field& field_info,
                  const core::FieldVariant& value) {
  return std::visit(
      ValidateTypeVisitor{.may_be_null = may_be_null, .field_info = field_info},
      value);
}

core::FieldVariant NormalizeFieldVariant(
    const core::Field::FieldType& field_type, const core::FieldVariant& value) {
  return std::visit(
      [&](auto&& arg) -> core::FieldVariant {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (deadfood::util::IsNumberT<T>::value) {
          switch (field_type) {
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
      value);
}

void CheckUniquenessConstraint(Database& db, const std::string& table_name,
                               const std::string& field_name,
                               const core::FieldVariant& value) {
  if (CountRowsWithMatchingField(db, table_name, field_name, value, 1) > 0) {
    throw std::runtime_error("unique constraint violated");
  }
}

void CheckForeignKeyConstraint(Database& db, const std::string& table_name,
                               const std::string& field_name,
                               const core::FieldVariant& value,
                               const Action& action) {
  for (const auto& constraint : db.constraints()) {
    if (auto c = std::get_if<core::ReferencesConstraint>(&constraint)) {
      if (c->master_table != table_name || c->master_field != field_name) {
        continue;
      }
      bool matches = false;
      switch (action) {
        case Action::Update:
          matches =
              c->on_update == core::ReferencesConstraint::OnAction::NoAction;
          break;
        case Action::Delete:
          matches =
              c->on_delete == core::ReferencesConstraint::OnAction::NoAction;
          break;
      }
      if (!matches) {
        continue;
      }
      if (CountRowsWithMatchingField(db, c->slave_table, c->slave_field, value,
                                     1) > 0) {
        throw std::runtime_error("foreign key constraint violated");
      }
    }
  }
}

}  // namespace deadfood::exec::util