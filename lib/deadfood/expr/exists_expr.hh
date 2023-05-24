#pragma once

#include <deadfood/expr/iexpr.hh>
#include <deadfood/scan/iscan.hh>

namespace deadfood::expr {

class ExistsExpr : public IExpr {
 public:
  ExistsExpr(std::unique_ptr<scan::IScan> scan);
  core::FieldVariant Eval() override;

 private:
  std::unique_ptr<scan::IScan> internal_;
};

}  // namespace deadfood::expr
