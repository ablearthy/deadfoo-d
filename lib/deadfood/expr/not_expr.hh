#pragma once

#include <deadfood/expr/iexpr.hh>
#include <deadfood/expr/bool_expr.hh>

namespace deadfood::expr {

class NotExpr : public IExpr {
 public:
  explicit NotExpr(std::unique_ptr<IExpr> internal);

  core::FieldVariant Eval() override;

 private:
  BoolExpr internal_;

};

}  // namespace deadfood
