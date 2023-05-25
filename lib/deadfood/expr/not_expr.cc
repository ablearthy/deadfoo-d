#include "not_expr.hh"

namespace deadfood::expr {

NotExpr::NotExpr(std::unique_ptr<IExpr> internal)
    : internal_{std::move(internal)} {}

core::FieldVariant NotExpr::Eval() {
  auto value = internal_.Eval();
  if (std::holds_alternative<core::null_t>(value)) {
    return core::null_t{};
  }
  return !std::get<bool>(value);
}

}  // namespace deadfood::expr