#pragma once

#include <deadfood/core/field.hh>

namespace deadfood::expr {

class IExpr {
 public:
  virtual core::FieldVariant Eval() = 0;

  virtual ~IExpr() = default;
};

}  // namespace deadfood::expr
