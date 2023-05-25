#include "is_expr.hh"

namespace deadfood::expr {

IsExpr::IsExpr(std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs)
    : lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

core::FieldVariant IsExpr::Eval() {
  const auto left = lhs_->Eval();
  const auto right = rhs_->Eval();
  return std::visit(
      [&](auto&& left_arg) {
        return std::visit(
            [&](auto&& right_arg) {
              using L = std::decay_t<decltype(left_arg)>;
              using R = std::decay_t<decltype(right_arg)>;
              if constexpr (std::is_same_v<L, R> &&
                            std::is_same_v<L, core::null_t>) {
                return true;
              } else if constexpr (std::is_same_v<L, R>) {
                return left_arg == right_arg;
              } else {
                return false;
              }
            },
            right);
      },
      left);
}

}  // namespace deadfood::expr