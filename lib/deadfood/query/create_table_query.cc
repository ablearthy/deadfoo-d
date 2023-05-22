#include "create_table_query.hh"

namespace deadfood::query {

CreateTableQuery::CreateTableQuery(const std::string& table_name)
    : table_name_{table_name} {}

bool CreateTableQuery::Contains(const std::string& field_name) const {
  return field_indices_.contains(field_name);
}

void CreateTableQuery::AddField(const std::string& field_name,
                                const core::Field& field, bool is_unique,
                                bool may_be_null) {
  if (field_indices_.contains(field_name)) {
    return;
  }
  size_t new_idx = field_names_.size();
  field_indices_.emplace(field_name, new_idx);
  field_names_.emplace_back(field_name);
  field_types_.emplace_back(field);
  is_unique_.emplace_back(is_unique);
  may_be_null_.emplace_back(may_be_null);
}

bool CreateTableQuery::IsUnique(const std::string& field_name) const {
  return is_unique_[field_indices_.at(field_name)];
}

bool CreateTableQuery::MayBeNull(const std::string& field_name) const {
  return may_be_null_[field_indices_.at(field_name)];
}

core::Field CreateTableQuery::GetField(const std::string& field_name) const {
  return field_types_[field_indices_.at(field_name)];
}

const std::vector<std::string>& CreateTableQuery::field_names() const {
  return field_names_;
}

}  // namespace deadfood::query