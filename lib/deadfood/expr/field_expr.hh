#pragma once

#include <memory>

#include <deadfood/scan/iscan.hh>
#include <deadfood/expr/iexpr.hh>

namespace deadfood::expr {

class FieldExpr : public IExpr {
 public:
  FieldExpr(scan::IScan* scan, const std::string& field_name);

  core::FieldVariant Eval() override;

 private:
  scan::IScan* scan_;
  const std::string& field_name_;
};

}  // namespace deadfood::expr
