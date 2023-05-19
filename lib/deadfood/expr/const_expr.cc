#include "const_expr.hh"

namespace deadfood::expr {

ConstExpr::ConstExpr(const core::FieldVariant& value) : value_{value} {}

deadfood::core::FieldVariant ConstExpr::Eval() { return value_; }

}  // namespace deadfood::expr
