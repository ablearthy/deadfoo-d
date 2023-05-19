#include "table_scan.hh"
#include "deadfood/core/row.hh"

namespace deadfood::scan {

TableScan::TableScan(storage::TableStorage& storage, const core::Schema& schema)
    : storage_{storage},
      schema_{schema},
      it_{storage.storage().begin()},
      cur_row_should_be_deleted_{false},
      before_start_{true} {}

void TableScan::BeforeFirst() {
  if (cur_row_should_be_deleted_) {
    storage_.storage().erase(it_);
    cur_row_should_be_deleted_ = false;
  }
  before_start_ = true;
}

bool TableScan::Next() {
  if (before_start_) {
    before_start_ = false;
    it_ = storage_.storage().begin();
    return true;
  }

  if (it_ == storage_.storage().end()) {
    return false;
  }

  auto new_it = it_;
  ++new_it;
  if (cur_row_should_be_deleted_) {
    storage_.storage().erase(it_);
    cur_row_should_be_deleted_ = false;
  }

  if (new_it == storage_.storage().end()) {
    return false;
  }

  it_ = new_it;
  return true;
}

bool TableScan::HasField(const std::string& field_name) const {
  return schema_.Exists(field_name);
}

core::FieldVariant TableScan::GetField(const std::string& field_name) const {
  const auto row = core::Row(it_->second, schema_);
  return row.GetField(field_name);
}

void TableScan::SetField(const std::string& field_name,
                         const core::FieldVariant& value) {
  auto row = core::Row(it_->second, schema_);
  row.SetField(field_name, value);
}

void TableScan::Insert() {
  if (cur_row_should_be_deleted_) {
    storage_.storage().erase(it_);
    cur_row_should_be_deleted_ = false;
  }

  size_t row_id = 0;
  if (!storage_.storage().empty()) {
    auto last = storage_.storage().end();
    --last;
    row_id = last->first + 1;
  }

  auto [new_it, _] =
      storage_.storage().emplace(row_id, storage::ByteBuffer(schema_.size()));
  it_ = new_it;
}

void TableScan::Delete() { cur_row_should_be_deleted_ = true; }

void TableScan::Close() {}

}  // namespace deadfood::scan