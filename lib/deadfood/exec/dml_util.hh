#pragma once

#include <deadfood/core/field.hh>
#include <deadfood/database.hh>

namespace deadfood::exec::util {

void ValidateType(bool may_be_null, const core::Field& field_info,
                  const core::FieldVariant& value);

core::FieldVariant NormalizeFieldVariant(
    const core::Field::FieldType& field_type, const core::FieldVariant& value);

void CheckUniquenessConstraint(Database& db, const std::string& table_name,
                               const std::string& field_name,
                               const core::FieldVariant& value);

}  // namespace deadfood::exec::util