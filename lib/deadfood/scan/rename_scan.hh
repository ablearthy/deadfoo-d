#pragma once

#include <deadfood/scan/iscan.hh>

namespace deadfood::scan {

class RenameScan : public IScan {
 public:
  RenameScan(std::unique_ptr<IScan> internal, const std::string& old_name,
             const std::string& new_name);
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
  std::string old_name_;
  std::string new_name_;
};

}  // namespace deadfood::scan
