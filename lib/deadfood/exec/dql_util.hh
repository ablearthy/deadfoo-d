#pragma once

#include <deadfood/database.hh>

namespace deadfood::exec::util {

size_t CountRowsWithMatchingField(Database& db, const std::string& table_name,
                                  const std::string& field_name,
                                  const core::FieldVariant& value,
                                  size_t limit = 0);
}