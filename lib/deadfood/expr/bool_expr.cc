#include "bool_expr.hh"

namespace deadfood::expr {

BoolExpr::BoolExpr(std::unique_ptr<IExpr> internal)
    : internal_{std::move(internal)} {}

BoolExpr::BoolExpr(BoolExpr&& other) noexcept
    : internal_{std::move(other.internal_)} {}

BoolExpr& BoolExpr::operator=(BoolExpr&& other) noexcept {
  if (this != &other) {
    internal_ = std::move(other.internal_);
  }
  return *this;
}

core::FieldVariant BoolExpr::Eval() {
  const auto val = internal_->Eval();
  return std::visit(
      [](auto&& arg) {
        using T = decltype(arg);

        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> ||
                      std::is_same_v<T, float> || std::is_same_v<T, double>) {
          return static_cast<bool>(arg != 0);
        } else if constexpr (std::is_same_v<T, std::span<char>>) {
          return static_cast<bool>(arg.size() != 0);
        } else {
          return false;
        }
      },
      val);
}

}  // namespace deadfood::expr