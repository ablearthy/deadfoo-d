#include "expr_convert.hh"
#include "cmp_expr.hh"
#include "is_expr.hh"
#include "not_expr.hh"

#include <deadfood/expr/field_expr.hh>
#include <deadfood/expr/const_expr.hh>
#include <deadfood/expr/bin_bool_expr.hh>
#include <deadfood/expr/math_expr.hh>

namespace deadfood::expr {

std::unique_ptr<IExpr> ConvertExprTreeToIExpr(const FactorTree& tree) {
  return nullptr;
}

ExprTreeConverter::ExprTreeConverter(std::unique_ptr<GetTableScan> scan)
    : get_table_scan_{std::move(scan)} {}

void RaiseIfNotBinFactors(const std::vector<expr::FactorTree>& factors) {
  if (factors.size() != 2) {
    throw std::runtime_error("can compare only two expressions");
  }
}

struct ExprTreeConverterVisitor {
  GetTableScan* get_table_scan;
  ExprTreeConverter& expr_tree_converter;

  std::unique_ptr<IExpr> operator()(const ExprTree& expr) {
    if (expr.factors.size() < 2) {
      throw std::runtime_error("invalid expr");
    }
    switch (expr.op) {
      case GenBinOp::Or:
        return GetBinBoolExpr(expr.factors, BinBoolOp::Or);
      case GenBinOp::And:
        return GetBinBoolExpr(expr.factors, BinBoolOp::And);
      case GenBinOp::Xor:
        return GetBinBoolExpr(expr.factors, BinBoolOp::Xor);
      case GenBinOp::Plus:
        return GetMathExpr(expr.factors, MathExprOp::Plus);
      case GenBinOp::Minus:
        return GetMathExpr(expr.factors, MathExprOp::Minus);
      case GenBinOp::Mul:
        return GetMathExpr(expr.factors, MathExprOp::Mul);
      case GenBinOp::Div:
        return GetMathExpr(expr.factors, MathExprOp::Div);
      case GenBinOp::Eq:
        RaiseIfNotBinFactors(expr.factors);
        return std::make_unique<CmpExpr>(
            CmpOp::Eq,
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]),
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1]));
      case GenBinOp::LT:
        RaiseIfNotBinFactors(expr.factors);
        return std::make_unique<CmpExpr>(
            CmpOp::Le,
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]),
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1]));
      case GenBinOp::LE:
        RaiseIfNotBinFactors(expr.factors);
        return std::make_unique<BinBoolExpr>(
            BinBoolOp::Or,
            std::make_unique<CmpExpr>(
                CmpOp::Le,
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]),
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1])),
            std::make_unique<CmpExpr>(
                CmpOp::Eq,
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]),
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1])));
      case GenBinOp::GE:
        RaiseIfNotBinFactors(expr.factors);
        return std::make_unique<BinBoolExpr>(
            BinBoolOp::Or,
            std::make_unique<CmpExpr>(
                CmpOp::Le,
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1]),
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0])),
            std::make_unique<CmpExpr>(
                CmpOp::Eq,
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]),
                expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1])));
      case GenBinOp::GT:
        RaiseIfNotBinFactors(expr.factors);
        return std::make_unique<CmpExpr>(
            CmpOp::Le,
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1]),
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]));
      case GenBinOp::Is:
        RaiseIfNotBinFactors(expr.factors);
        return std::make_unique<IsExpr>(
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]),
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1]));
      case GenBinOp::IsNot:
        RaiseIfNotBinFactors(expr.factors);
        return std::make_unique<NotExpr>(std::make_unique<IsExpr>(
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[0]),
            expr_tree_converter.ConvertExprTreeToIExpr(expr.factors[1])));
    }
  }

  std::unique_ptr<IExpr> operator()(const std::string& field_name) {
    return std::make_unique<FieldExpr>(get_table_scan->GetScan(field_name),
                                       field_name);
  }
  std::unique_ptr<IExpr> operator()(const Constant& constant) {
    core::FieldVariant var = std::visit(
        [&](auto&& arg) -> core::FieldVariant { return arg; }, constant);
    return std::make_unique<ConstExpr>(std::move(var));
  }

 private:
  std::unique_ptr<IExpr> GetBinBoolExpr(const std::vector<FactorTree>& factors,
                                        BinBoolOp op) {
    std::unique_ptr<IExpr> ret = std::make_unique<BinBoolExpr>(
        op, expr_tree_converter.ConvertExprTreeToIExpr(factors[0]),
        expr_tree_converter.ConvertExprTreeToIExpr(factors[1]));

    for (size_t i = 2; i < factors.size(); ++i) {
      std::unique_ptr<IExpr> tmp = std::make_unique<BinBoolExpr>(
          op, std::move(ret),
          expr_tree_converter.ConvertExprTreeToIExpr(factors[i]));
      ret = std::move(tmp);
    }
    return ret;
  }
  std::unique_ptr<IExpr> GetMathExpr(const std::vector<FactorTree>& factors,
                                     MathExprOp op) {
    std::unique_ptr<IExpr> ret = std::make_unique<MathExpr>(
        op, expr_tree_converter.ConvertExprTreeToIExpr(factors[0]),
        expr_tree_converter.ConvertExprTreeToIExpr(factors[1]));
    for (size_t i = 2; i < factors.size(); ++i) {
      std::unique_ptr<IExpr> tmp = std::make_unique<MathExpr>(
          op, std::move(ret),
          expr_tree_converter.ConvertExprTreeToIExpr(factors[i]));
      ret = std::move(tmp);
    }
    return ret;
  }
};

std::unique_ptr<IExpr> ExprTreeConverter::ConvertExprTreeToIExpr(
    const FactorTree& tree) {
  auto vis = ExprTreeConverterVisitor{get_table_scan_.get(), *this};
  auto expr = std::visit(vis, tree.factor);
  if (tree.neg_applied) {
    auto tmp = std::make_unique<MathExpr>(
        MathExprOp::Minus, std::make_unique<ConstExpr>(0), std::move(expr));
    expr = std::move(tmp);
  }
  if (tree.not_applied) {
    auto tmp = std::make_unique<NotExpr>(std::move(expr));
    expr = std::move(tmp);
  }
  return expr;
}
}  // namespace deadfood::expr
