#include "math_expr.hh"

#include <deadfood/util/is_number_t.hh>

namespace deadfood::expr {
MathExpr::MathExpr(MathExprOp op, std::unique_ptr<IExpr> lhs,
                   std::unique_ptr<IExpr> rhs)
    : op_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

bool IsNumber(const core::FieldVariant& variant) {
  return std::visit(
      [](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (deadfood::util::IsNumberT<T>::value) {
          return true;
        }
        return false;
      },
      variant);
}

core::FieldVariant MathExpr::Eval() {
  auto lhs = lhs_->Eval();
  auto rhs = rhs_->Eval();

  return std::visit(
      [&](auto&& lhs_arg) -> core::FieldVariant {
        using L = std::decay_t<decltype(lhs_arg)>;
        return std::visit(
            [&](auto&& rhs_arg) -> core::FieldVariant {
              using R = std::decay_t<decltype(rhs_arg)>;
              if constexpr (std::is_same_v<L, core::null_t> ||
                            std::is_same_v<R, core::null_t>) {
                return core::null_t{};
              } else if constexpr (deadfood::util::IsNumberT<L>::value &&
                                   deadfood::util::IsNumberT<R>::value) {
                switch (op_) {
                  case MathExprOp::Plus:
                    return lhs_arg + rhs_arg;
                  case MathExprOp::Minus:
                    return lhs_arg - rhs_arg;
                  case MathExprOp::Mul:
                    return lhs_arg * rhs_arg;
                  case MathExprOp::Div:
                    return lhs_arg / rhs_arg;
                }
              } else if constexpr (std::is_same_v<L, std::string> &&
                                   std::is_same_v<R, std::string>) {
                switch (op_) {
                  case MathExprOp::Plus:
                    return lhs_arg + rhs_arg;
                  case MathExprOp::Minus:
                    throw std::runtime_error(
                        "cannot perform `-` operation on strings");
                  case MathExprOp::Mul:
                    throw std::runtime_error(
                        "cannot perform `*` operation on strings");
                  case MathExprOp::Div:
                    throw std::runtime_error(
                        "cannot perform `/` operation on strings");
                }
              }
              throw std::runtime_error("cannot perform operation");
            },
            rhs);
      },
      lhs);
}
}  // namespace deadfood::expr