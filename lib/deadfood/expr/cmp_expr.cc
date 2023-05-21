#include "cmp_expr.hh"

namespace deadfood::expr {

CmpExpr::CmpExpr(CmpOp op, std::unique_ptr<IExpr> lhs,
                 std::unique_ptr<IExpr> rhs)
    : op_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

template <typename L>
bool CompareTrivial(L left, CmpOp op, const core::FieldVariant& rhs) {
  return std::visit(
      [&](auto&& right) {
        using T = std::decay_t<decltype(right)>;
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> ||
                      std::is_same_v<T, float> || std::is_same_v<T, double>) {
          switch (op) {
            case CmpOp::Eq:
              return left == right;
            case CmpOp::Le:
              return left < right;
          }
        }
        return false;
      },
      rhs);
}

bool CompareVarcharEq(std::span<char> left, std::span<char> right) {
  if (left.size() != right.size()) {
    return false;
  }
  const auto left_data = left.data();
  const auto right_data = right.data();
  for (size_t idx = 0; idx < left.size(); ++idx) {
    if (left_data[idx] != right_data[idx]) {
      return false;
    }
  }
  return true;
}

bool CompareVarcharLe(std::span<char> left, std::span<char> right) {
  if (left.size() > right.size()) {
    return false;
  }
  const auto left_data = left.data();
  const auto right_data = right.data();
  for (size_t idx = 0; idx < left.size(); ++idx) {
    if (left_data[idx] >= right_data[idx]) {
      return false;
    }
  }
  return true;
}

core::FieldVariant CmpExpr::Eval() {
  const auto lhs = lhs_->Eval();
  const auto rhs = rhs_->Eval();

  return std::visit(
      [&](auto&& left) {
        using T = std::decay_t<decltype(left)>;
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> ||
                      std::is_same_v<T, float> || std::is_same_v<T, double>) {
          return CompareTrivial(left, op_, rhs);
        } else if constexpr (std::is_same_v<T, std::span<char>>) {
          if (std::holds_alternative<std::span<char>>(rhs)) {
            const auto right = std::get<std::span<char>>(rhs);
            switch (op_) {
              case CmpOp::Eq:
                return CompareVarcharEq(left, right);
              case CmpOp::Le:
                return CompareVarcharLe(left, right);
            }
          }
          return false;
        } else if constexpr (std::is_same_v<T, core::null_t>) {
          return std::holds_alternative<core::null_t>(rhs);
        }
        return false;
      },
      lhs);
}

}  // namespace deadfood::expr