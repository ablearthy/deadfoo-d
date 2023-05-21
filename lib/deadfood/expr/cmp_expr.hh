#pragma once

#include <deadfood/expr/iexpr.hh>

namespace deadfood::expr {

enum class CmpOp { Eq, Le };

class CmpExpr : public IExpr {
 public:
  CmpExpr(CmpOp op, std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs);

  CmpExpr(CmpExpr&& other) noexcept;
  CmpExpr& operator=(CmpExpr&& other) noexcept;

  core::FieldVariant Eval() override;

  ~CmpExpr() override = default;

 private:
  CmpOp op_;
  std::unique_ptr<IExpr> lhs_;
  std::unique_ptr<IExpr> rhs_;
};

}  // namespace deadfood::expr
