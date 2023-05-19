#pragma once

#include <deadfood/core/field.hh>

namespace deadfood::scan {


class IScan {
 public:
  virtual void BeforeFirst() = 0;
  virtual bool Next() = 0;

  [[nodiscard]] virtual bool HasField(const std::string& field_name) const = 0;
  [[nodiscard]] virtual core::FieldVariant GetField(const std::string& field_name) const = 0;
  virtual void SetField(const std::string& field_name, const core::FieldVariant& value) = 0;

  virtual void Insert() = 0;
  virtual void Delete() = 0;

  virtual void Close() = 0;

  virtual ~IScan() = default;
};

}  // namespace deadfood::scan