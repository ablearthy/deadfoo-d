#pragma once

#include <deadfood/scan/iscan.hh>

namespace deadfood::scan {

class ProductScan : public IScan {
 public:
  ProductScan(std::unique_ptr<IScan> lhs, std::unique_ptr<IScan> rhs);

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
  std::unique_ptr<IScan> lhs_;
  std::unique_ptr<IScan> rhs_;
  bool lhs_has_rows;
};

}  // namespace deadfood::scan
