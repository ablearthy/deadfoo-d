#pragma once

#include <deadfood/expr/iexpr.hh>

namespace deadfood::expr {

class ConstExpr : public IExpr {
 public:
  explicit ConstExpr(const core::FieldVariant& value);

  core::FieldVariant Eval() override;

  ~ConstExpr() override = default;
 private:
  core::FieldVariant value_;
};

}  // namespace deadfood::expr
