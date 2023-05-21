#pragma once

#include <deadfood/scan/iscan.hh>
#include <unordered_set>

namespace deadfood::scan {

class ProjectScan : public IScan {
 public:
  ProjectScan(std::unique_ptr<IScan> internal, const std::unordered_set<std::string>& fields);

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
  std::unordered_set<std::string> fields_;
};

}  // namespace deadfood::scan
