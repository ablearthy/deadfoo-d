#pragma once

#include <deadfood/core/schema.hh>
#include <deadfood/storage/byte_buffer.hh>

namespace deadfood::core {

class Row {
 public:
  Row(storage::ByteBuffer& storage, const Schema& schema)
      : storage_{storage}, schema_{schema} {}

  [[nodiscard]] FieldVariant GetField(const std::string& field_name) const;
  void SetField(const std::string& field_name, const FieldVariant& value);

  [[nodiscard]] bool IsNull(const std::string& field_name) const;

 private:
  storage::ByteBuffer& storage_;
  const Schema& schema_;
};

}  // namespace deadfood::core
