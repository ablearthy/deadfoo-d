#pragma once

#include <deadfood/scan/iscan.hh>
#include <deadfood/expr/iexpr.hh>

namespace deadfood::scan {

class ExtendScan : public IScan {
 public:
  ExtendScan(std::unique_ptr<IScan> internal, std::unique_ptr<expr::IExpr> expr, const std::string& name);
  void BeforeFirst() override;
  bool Next() override;
  bool HasField(const std::string& field_name) const override;
  core::FieldVariant GetField(const std::string& field_name) const override;
  void SetField(const std::string& field_name,
                const core::FieldVariant& value) override;
  void Insert() override;
  void Delete() override;
  void Close() override;

 private:
  std::unique_ptr<IScan> internal_;
  std::unique_ptr<expr::IExpr> expr_;
  std::string name_;
};

}  // namespace deadfood::scan
