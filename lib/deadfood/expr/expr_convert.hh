#pragma once

#include <deadfood/scan/iscan.hh>
#include <deadfood/expr/iexpr.hh>
#include <deadfood/expr/expr_tree.hh>

namespace deadfood::expr {

class GetTableScan {
 public:
  virtual scan::IScan* GetScan(const std::string& field_name) = 0;

  virtual ~GetTableScan() = default;
};

class ExprTreeConverter {
 public:
  explicit ExprTreeConverter(std::unique_ptr<GetTableScan> scan);

  std::unique_ptr<IExpr> ConvertExprTreeToIExpr(const FactorTree& tree);

 private:
  std::unique_ptr<GetTableScan> get_table_scan_;
};

}  // namespace deadfood::expr