#pragma once

#include <map>
#include <vector>
#include <string>

#include "deadfood/core/field.hh"

namespace deadfood::core {

class Schema {
 public:
  Schema();

  size_t size() const;

  [[nodiscard]] const std::vector<std::string>& fields() const;
  [[nodiscard]] const Field& field_info(const std::string& field_name) const;
  [[nodiscard]] size_t data_offset() const;

  [[nodiscard]] size_t Index(const std::string& field_name) const;
  [[nodiscard]] size_t Offset(const std::string& field_name) const;
  [[nodiscard]] bool Exists(const std::string& field_name) const;

  [[nodiscard]] bool MayBeNull(const std::string& field_name) const;
  [[nodiscard]] bool IsUnique(const std::string& field_name) const;



  void AddField(const std::string& field_name, const Field& field, bool may_be_null, bool is_unique);

 private:
  std::vector<std::string> fields_;
  std::vector<size_t> offsets_;
  std::map<std::string, size_t> offset_map_;
  std::map<std::string, Field> field_info_;
  std::map<std::string, size_t> indices_;
  std::map<std::string, bool> may_be_null_;
  std::map<std::string, bool> is_unique_;

  size_t data_offset_;
};

}  // namespace deadfood::core