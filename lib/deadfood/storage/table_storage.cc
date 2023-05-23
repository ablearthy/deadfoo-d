#include "table_storage.hh"

namespace deadfood::storage {

ByteBuffer& TableStorage::GetByRowId(size_t row_id) {
  return storage_.at(row_id);
}

bool TableStorage::Exists(size_t row_id) const {
  return storage_.contains(row_id);
}

std::map<size_t, ByteBuffer>& TableStorage::storage() { return storage_; }
const std::map<size_t, ByteBuffer>& TableStorage::storage_const() const {
  return storage_;
}

}  // namespace deadfood::storage