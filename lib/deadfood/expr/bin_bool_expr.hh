#pragma once

#include <deadfood/expr/iexpr.hh>
#include <deadfood/expr/bool_expr.hh>

namespace deadfood::expr {

enum class BinBoolOp { And, Or, Xor };

class BinBoolExpr : public IExpr {
 public:
  BinBoolExpr(BinBoolOp op, std::unique_ptr<IExpr> lhs,
              std::unique_ptr<IExpr> rhs);

  core::FieldVariant Eval() override;
  ~BinBoolExpr() override = default;

 private:
  BinBoolOp op_;
  BoolExpr lhs_;
  BoolExpr rhs_;
};

}  // namespace deadfood::expr
