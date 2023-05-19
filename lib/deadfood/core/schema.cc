#include "schema.hh"

namespace deadfood::core {

Schema::Schema() : data_offset_{1} {}

const std::vector<std::string>& Schema::fields() const { return fields_; }

const Field& Schema::field_info(const std::string& field_name) const {
  return field_info_.at(field_name);
}

size_t Schema::data_offset() const { return data_offset_; }

size_t Schema::Index(const std::string& field_name) const {
  return indices_.at(field_name);
}

size_t Schema::Offset(const std::string& field_name) const {
  return offset_map_.at(field_name) + data_offset();
}

bool Schema::Exists(const std::string& field_name) const {
  return field_info_.contains(field_name);
}

void Schema::AddField(const std::string& field_name, const Field& field) {
  if (Exists(field_name)) {
    return;
  }

  indices_.emplace(field_name, fields_.size());
  fields_.emplace_back(field_name);
  field_info_.emplace(field_name, field);

  if (fields_.size() > 8 * data_offset_) {
    ++data_offset_;
  }

  if (offsets_.empty()) {
    offsets_.emplace_back(0);
  } else {
    offsets_.emplace_back(offsets_.back() + field.size());
  }
  offset_map_.emplace(field_name, offsets_.back());
}

size_t Schema::size() const {
  if (offsets_.empty()) {
    return 0;
  }
  const auto last_offset = Offset(fields_.back());
  const auto last_field_size = field_info_.at(fields_.back()).size();
  return last_offset + last_field_size;
}

}  // namespace deadfood::core