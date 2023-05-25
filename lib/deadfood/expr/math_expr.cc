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

  if (std::holds_alternative<core::null_t>(lhs) ||
      std::holds_alternative<core::null_t>(rhs)) {
    return core::null_t{};
  }

  if (IsNumber(lhs) && IsNumber(rhs)) {
    return std::visit(
        [&](auto&& lhs_arg) -> core::FieldVariant {
          if constexpr (deadfood::util::IsNumberT<
                            std::decay_t<decltype(lhs_arg)>>::value) {
            return std::visit(
                [&](auto&& rhs_arg) -> core::FieldVariant {
                  if constexpr (deadfood::util::IsNumberT<
                                    std::decay_t<decltype(rhs_arg)>>::value) {
                    switch (op_) {
                      case MathExprOp::Plus:
                        return lhs_arg + lhs_arg;
                      case MathExprOp::Minus:
                        return lhs_arg - lhs_arg;
                      case MathExprOp::Mul:
                        return lhs_arg * lhs_arg;
                      case MathExprOp::Div:
                        return lhs_arg / lhs_arg;
                    }
                  }
                  return 0;
                },
                rhs);
          }
          return 0;
        },
        lhs);
  }

  switch (op_) {
    case MathExprOp::Plus:
      if (std::holds_alternative<std::string>(lhs) &&
          std::holds_alternative<std::string>(rhs)) {
        return std::get<std::string>(lhs) + std::get<std::string>(rhs);
      }
      throw std::runtime_error("cannot perform `+` operation");
    case MathExprOp::Minus:
      throw std::runtime_error("cannot perform `-` operation");
    case MathExprOp::Mul:
      throw std::runtime_error("cannot perform `*` operation");
    case MathExprOp::Div:
      throw std::runtime_error("cannot perform `/` operation");
  }
}
}  // namespace deadfood::expr