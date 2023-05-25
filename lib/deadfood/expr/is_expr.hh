#pragma once

#include <deadfood/expr/iexpr.hh>

namespace deadfood::expr {

class IsExpr : public IExpr {
 public:
  IsExpr(std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs);
  core::FieldVariant Eval() override;

 private:
  std::unique_ptr<IExpr> lhs_;
  std::unique_ptr<IExpr> rhs_;
};

}  // namespace deadfood
