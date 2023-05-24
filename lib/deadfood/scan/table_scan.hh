#pragma once

#include <deadfood/scan/iscan.hh>
#include <deadfood/storage/table_storage.hh>
#include "deadfood/core/schema.hh"

namespace deadfood::scan {

class TableScan : public IScan {
 public:
  TableScan(storage::TableStorage& storage, const core::Schema& schema,
            const std::string& table_name);

  void set_table_name(const std::string& table_name);

  void BeforeFirst() override;
  bool Next() override;

  [[nodiscard]] bool HasField(const std::string& field_name) const override;
  [[nodiscard]] core::FieldVariant GetField(
      const std::string& field_name) const override;
  void SetField(const std::string& field_name,
                const core::FieldVariant& value) override;

  void Insert() override;
  void Delete() override;

  void Close() override;

 private:
  std::string table_name_;
  storage::TableStorage& storage_;
  const core::Schema& schema_;
  decltype(storage_.storage().begin()) it_;
  bool cur_row_should_be_deleted_;
  bool before_start_;

  [[nodiscard]] std::string NormalizeFieldName(
      const std::string& field_name) const;
};

}  // namespace deadfood::scan