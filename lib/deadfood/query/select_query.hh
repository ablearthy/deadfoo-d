#pragma once

#include <variant>
#include <string>
#include <optional>
#include <vector>

#include <deadfood/expr/expr_tree.hh>

namespace deadfood::query {

struct SelectQuery;

struct FromTable {
  std::string table_name;
  std::optional<std::string> renamed;
};

enum class JoinType { Left, Right, Inner };

struct Join {
  JoinType type;
  std::string table_name;
  std::string alias;
  expr::FactorTree predicate;
};

struct SelectAllSelector {};

struct FieldSelector {
  expr::FactorTree expr;
  std::string field_name;
};

using SelectFrom = std::variant<SelectQuery, FromTable>;
using Selector = std::variant<SelectAllSelector, std::string, FieldSelector>;

struct SelectQuery {
  std::vector<Selector> selectors;
  std::vector<SelectFrom> sources;
  std::optional<expr::FactorTree> predicate;
  std::vector<Join> joins;
};

}  // namespace deadfood::query
