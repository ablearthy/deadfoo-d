#pragma once

#include <string>
#include <deadfood/expr/expr_tree.hh>
#include <map>
#include <optional>

namespace deadfood::query {

struct UpdateQuery {
  std::string table_name;
  std::map<std::string, expr::FactorTree> sets;
  std::optional<expr::FactorTree> predicate;
};

}  // namespace deadfood::query