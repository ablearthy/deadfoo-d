#include "field.hh"

namespace deadfood::core {

const Field::FieldType& Field::type() const { return type_; }

const size_t& Field::size() const { return size_; }

Field field::VarcharField(size_t size) {
  return Field(Field::FieldType::Varchar, size);
}

}  // namespace deadfood::core