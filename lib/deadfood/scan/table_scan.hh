#pragma once

#include <deadfood/scan/iscan.hh>
#include <deadfood/storage/table_storage.hh>
#include "deadfood/core/schema.hh"

namespace deadfood::scan {

class TableScan : public IScan {
 public:
  TableScan(storage::TableStorage& storage, const core::Schema& schema);

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
  storage::TableStorage& storage_;
  const core::Schema& schema_;
  decltype(storage_.storage().begin()) it_;
  bool cur_row_should_be_deleted_;
  bool before_start_;
};

}  // namespace deadfood::scan