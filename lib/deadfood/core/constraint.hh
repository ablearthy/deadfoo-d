#pragma once

#include <variant>
#include <string>

namespace deadfood::core {

struct ReferencesConstraint {
  enum class OnAction { NoAction, Cascade, SetNull, SetDefault };
  std::string master_table;
  std::string master_field;
  std::string slave_table;
  std::string slave_field;
  OnAction on_delete;
  OnAction on_update;
};

using Constraint = std::variant<ReferencesConstraint>;

}  // namespace deadfood::core
