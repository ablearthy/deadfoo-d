#pragma once

#include <deadfood/expr/iexpr.hh>

namespace deadfood::expr {

class BoolExpr : public IExpr {
 public:
  explicit BoolExpr(std::unique_ptr<IExpr> internal);

  core::FieldVariant Eval() override;

 private:
  std::unique_ptr<IExpr> internal_;
};

}  // namespace deadfood::expr
