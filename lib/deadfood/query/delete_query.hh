#pragma once

#include <deadfood/expr/expr_tree.hh>
#include <optional>
#include <string>

namespace deadfood::query {

struct DeleteQuery {
  std::string table_name;
  std::optional<expr::FactorTree> predicate;
};

}  // namespace deadfood::query