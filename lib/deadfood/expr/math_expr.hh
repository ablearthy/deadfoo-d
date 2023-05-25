#pragma once

#include <deadfood/expr/iexpr.hh>

namespace deadfood::expr {

enum class MathExprOp { Plus, Minus, Mul, Div };

class MathExpr : public IExpr {
 public:
  MathExpr(MathExprOp op, std::unique_ptr<IExpr> lhs,
           std::unique_ptr<IExpr> rhs);

  core::FieldVariant Eval() override;

 private:
  MathExprOp op_;
  std::unique_ptr<IExpr> lhs_;
  std::unique_ptr<IExpr> rhs_;
};

}  // namespace deadfood::expr
