#include "cmp_expr.hh"

namespace deadfood::expr {

CmpExpr::CmpExpr(CmpOp op, std::unique_ptr<IExpr> lhs,
                 std::unique_ptr<IExpr> rhs)
    : op_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

CmpExpr::CmpExpr(CmpExpr&& other) noexcept
    : op_{other.op_},
      lhs_{std::move(other.lhs_)},
      rhs_{std::move(other.rhs_)} {}

CmpExpr& CmpExpr::operator=(CmpExpr&& other) noexcept {
  lhs_ = std::move(other.lhs_);
  rhs_ = std::move(other.rhs_);
  op_ = other.op_;
  return *this;
}

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
        throw std::runtime_error("cannot compare number and not number");
        return false;
      },
      rhs);
}

bool CompareVarcharEq(const std::string& left, const std::string& right) {
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

bool CompareVarcharLe(const std::string& left, const std::string& right) {
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
        } else if constexpr (std::is_same_v<T, std::string>) {
          if (std::holds_alternative<std::string>(rhs)) {
            const auto right = std::get<std::string>(rhs);
            switch (op_) {
              case CmpOp::Eq:
                return CompareVarcharEq(left, right);
              case CmpOp::Le:
                return CompareVarcharLe(left, right);
            }
          }
          throw std::runtime_error("cannot compare string and not string");
          return false;
        } else if constexpr (std::is_same_v<T, core::null_t>) {
          return std::holds_alternative<core::null_t>(rhs);
        }
        return false;
      },
      lhs);
}

}  // namespace deadfood::expr