#include "bin_bool_expr.hh"

namespace deadfood::expr {

BinBoolExpr::BinBoolExpr(BinBoolOp op, std::unique_ptr<IExpr> lhs,
                         std::unique_ptr<IExpr> rhs)
    : op_{op}, lhs_{BoolExpr(std::move(lhs))}, rhs_{BoolExpr(std::move(rhs))} {}

core::FieldVariant BinBoolExpr::Eval() {
  const auto left = std::get<bool>(lhs_.Eval());
  const auto right = std::get<bool>(rhs_.Eval());
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