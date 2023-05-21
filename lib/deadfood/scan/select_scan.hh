#pragma once

#include <deadfood/scan/iscan.hh>
#include <deadfood/expr/bool_expr.hh>

namespace deadfood::scan {

class SelectScan : public IScan {
 public:

  SelectScan(std::unique_ptr<IScan> scan, expr::BoolExpr&& predicate);

  void set_predicate(expr::BoolExpr&& predicate);

  void BeforeFirst() override;

  bool Next() override;

  [[nodiscard]] bool HasField(const std::string& field_name) const override;
  [[nodiscard]] core::FieldVariant GetField(const std::string& field_name) const override;
  void SetField(const std::string& field_name,
                const core::FieldVariant& value) override;

  void Insert() override;
  void Delete() override;

  void Close() override;

  ~SelectScan() override = default;

 private:
  std::unique_ptr<IScan> internal_;
  expr::BoolExpr predicate_;
};

}  // namespace deadfood::scan
