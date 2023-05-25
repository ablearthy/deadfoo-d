#include "bin_bool_expr.hh"

namespace deadfood::expr {

BinBoolExpr::BinBoolExpr(BinBoolOp op, std::unique_ptr<IExpr> lhs,
                         std::unique_ptr<IExpr> rhs)
    : op_{op}, lhs_{BoolExpr(std::move(lhs))}, rhs_{BoolExpr(std::move(rhs))} {}

core::FieldVariant BinBoolExpr::Eval() {
  const auto left_eval = lhs_.Eval();
  const auto right_eval = rhs_.Eval();

  if (std::holds_alternative<core::null_t>(left_eval) &&
      std::holds_alternative<core::null_t>(right_eval)) {
    return core::null_t{};
  }

  if (std::holds_alternative<core::null_t>(left_eval)) {
    switch (op_) {
      case BinBoolOp::Xor:
      case BinBoolOp::And:
        return core::null_t{};
      case BinBoolOp::Or:
        return std::get<bool>(right_eval);
    }
  }

  if (std::holds_alternative<core::null_t>(right_eval)) {
    switch (op_) {
      case BinBoolOp::Xor:
      case BinBoolOp::And:
        return core::null_t{};
      case BinBoolOp::Or:
        return std::get<bool>(left_eval);
    }
  }

  const auto left = std::get<bool>(left_eval);
  const auto right = std::get<bool>(right_eval);
  switch (op_) {
    case BinBoolOp::And:
      return left && right;
    case BinBoolOp::Or:
      return left || right;
    case BinBoolOp::Xor:
      return left ^ right;
  }
}

}  // namespace deadfood::expr