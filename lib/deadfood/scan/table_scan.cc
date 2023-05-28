#include "table_scan.hh"

#include <deadfood/core/row.hh>
#include <deadfood/util/parse.hh>

namespace deadfood::scan {

TableScan::TableScan(storage::TableStorage& storage, const core::Schema& schema,
                     const std::string& table_name)
    : table_name_{table_name},
      storage_{storage},
      schema_{schema},
      it_{storage.storage().begin()},
      before_start_{true} {}

void TableScan::set_table_name(const std::string& table_name) {
  table_name_ = table_name;
}

void TableScan::BeforeFirst() { before_start_ = true; }

bool TableScan::Next() {
  if (before_start_) {
    before_start_ = false;
    it_ = storage_.storage().begin();
    return it_ != storage_.storage().end();
  }

  if (it_ == storage_.storage().end()) {
    return false;
  }

  ++it_;

  return it_ != storage_.storage().end();
}

bool TableScan::HasField(const std::string& field_name) const {
  const auto [tbl, field] = deadfood::parse::util::GetFullFieldName(field_name);
  if (tbl.has_value() && tbl.value() != table_name_) {
    return false;
  }
  return schema_.Exists(field);
}

core::FieldVariant TableScan::GetField(const std::string& field_name) const {
  if (it_ == storage_.storage().end()) {
    return core::null_t{};
  }
  const auto row = core::Row(it_->second, schema_);
  const auto [_, field] = deadfood::parse::util::GetFullFieldName(field_name);
  return row.GetField(field);
}

void TableScan::SetField(const std::string& field_name,
                         const core::FieldVariant& value) {
  auto row = core::Row(it_->second, schema_);
  const auto [_, field] = deadfood::parse::util::GetFullFieldName(field_name);
  row.SetField(field, value);
}

void TableScan::Insert() {
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

void TableScan::Delete() {
  if (before_start_ || storage_.storage().empty() ||
      it_ == storage_.storage().end()) {
    return;
  }

  auto copy_it = it_;
  if (storage_.storage().begin() == it_) {
    ++copy_it;
  } else {
    --copy_it;
  }

  storage_.storage().erase(it_);
  it_ = copy_it;
}

void TableScan::Close() {}

}  // namespace deadfood::scan