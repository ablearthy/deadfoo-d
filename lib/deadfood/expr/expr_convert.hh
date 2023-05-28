#pragma once

#include <deadfood/scan/iscan.hh>
#include <deadfood/expr/iexpr.hh>
#include <deadfood/expr/expr_tree.hh>
#include <deadfood/expr/scan_selector/iscanselector.hh>

namespace deadfood::expr {

class ExprTreeConverter {
 public:
  explicit ExprTreeConverter(std::unique_ptr<IScanSelector> scan);

  std::unique_ptr<IExpr> ConvertExprTreeToIExpr(const FactorTree& tree);

 private:
  std::unique_ptr<IScanSelector> get_table_scan_;
};

}  // namespace deadfood::expr