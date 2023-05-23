#pragma once

#include <map>

#include <deadfood/storage/byte_buffer.hh>

namespace deadfood::storage {

class TableStorage {
 public:
  ByteBuffer& GetByRowId(size_t row_id);

  [[nodiscard]] bool Exists(size_t row_id) const;

  std::map<size_t, ByteBuffer>& storage();
  [[nodiscard]] const std::map<size_t, ByteBuffer>& storage_const() const;



 private:
  std::map<size_t, ByteBuffer> storage_;
};

}  // namespace deadfood::storage
