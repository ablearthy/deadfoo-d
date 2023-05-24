#pragma once

#include <optional>

#include <deadfood/expr/expr_tree.hh>

namespace deadfood::query {

struct InsertQuery {
  std::string table_name;
  std::optional<std::vector<std::string>> fields;
  std::vector<std::vector<expr::FactorTree>> values;
};

}  // namespace deadfood::query