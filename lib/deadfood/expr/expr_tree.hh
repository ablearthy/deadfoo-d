#pragma once

#include <vector>
#include <variant>
#include <string>
#include <deadfood/core/field.hh>

namespace deadfood::expr {

enum class GenBinOp { Or, And, Xor, Plus, Minus, Mul, Div, Eq, LT, LE, GE, GT };

using Constant = std::variant<int, double, std::string, bool, core::null_t>;

struct FactorTree;

struct ExprTree {
  GenBinOp op;
  std::vector<FactorTree> factors;
};

struct FactorTree {
  bool neg_applied;
  bool not_applied;
  std::variant<ExprTree, std::string, Constant> factor;
};

}  // namespace deadfood::expr