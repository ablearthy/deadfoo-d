#pragma once

#include <string>
#include <vector>
#include <map>

#include <deadfood/core/field.hh>

namespace deadfood::query {

class CreateTableQuery {
 public:
  explicit CreateTableQuery(const std::string& table_name);

  const std::string& table_name() const;

  void AddField(const std::string& field_name, const core::Field& field,
                bool is_unique, bool may_be_null);

  [[nodiscard]] bool Contains(const std::string& field_name) const;
  [[nodiscard]] bool IsUnique(const std::string& field_name) const;
  [[nodiscard]] bool MayBeNull(const std::string& field_name) const;

  [[nodiscard]] core::Field GetField(const std::string& field_name) const;

  [[nodiscard]] const std::vector<std::string>& field_names() const;

 private:
  std::string table_name_;

  std::map<std::string, size_t> field_indices_;
  std::vector<std::string> field_names_;
  std::vector<core::Field> field_types_;
  std::vector<bool> is_unique_;
  std::vector<bool> may_be_null_;
};

}  // namespace deadfood::query
