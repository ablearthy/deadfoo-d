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

  void AddField(const std::string& field_name, const Field& field);

 private:
  std::vector<std::string> fields_;
  std::vector<size_t> offsets_;
  std::map<std::string, size_t> offset_map_;
  std::map<std::string, Field> field_info_;
  std::map<std::string, size_t> indices_;
  size_t data_offset_;
};

}  // namespace deadfood::core