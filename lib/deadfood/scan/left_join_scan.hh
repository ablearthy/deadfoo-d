#pragma once

#include <deadfood/scan/iscan.hh>
#include <deadfood/expr/bool_expr.hh>

namespace deadfood::scan {

class LeftJoinScan : public IScan {
 public:
  LeftJoinScan(std::unique_ptr<IScan> lhs, std::unique_ptr<IScan> rhs, std::unique_ptr<expr::IExpr> expr);

  void BeforeFirst() override;
  bool Next() override;
  bool HasField(const std::string& field_name) const override;
  core::FieldVariant GetField(const std::string& field_name) const override;
  void SetField(const std::string& field_name,
                const core::FieldVariant& value) override;
  void Insert() override;
  void Delete() override;
  void Close() override;

  void set_predicate(std::unique_ptr<expr::IExpr> expr);

 private:
  std::unique_ptr<IScan> lhs_;
  std::unique_ptr<IScan> rhs_;
  expr::BoolExpr predicate_;
  bool lhs_has_row_;
  bool cur_lhs_has_rhs_;
  bool rhs_null_;
};

}  // namespace deadfood::scan
