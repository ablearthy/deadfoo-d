#pragma once

#include <cstddef>
#include <variant>
#include <memory>

namespace deadfood::core {

class Field {
 public:
  enum class FieldType { Bool, Int, Float, Double, Varchar };

  constexpr Field(FieldType type, size_t size) : type_{type}, size_{size} {}

  const FieldType& type() const;
  const size_t& size() const;

 private:
  FieldType type_;
  size_t size_;
};

struct null_t{};

using FieldVariant =
    std::variant<bool, int, float, double, std::unique_ptr<char[]>, null_t>;

namespace field {

constexpr Field kIntField = Field(Field::FieldType::Int, 4);
constexpr Field kBoolField = Field(Field::FieldType::Bool, 4);
constexpr Field kFloatField = Field(Field::FieldType::Float, 4);
constexpr Field kDoubleField = Field(Field::FieldType::Double, 8);

Field VarcharField(size_t size);

}  // namespace field

}  // namespace deadfood::core
