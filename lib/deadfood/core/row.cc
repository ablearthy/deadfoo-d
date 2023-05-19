#include "row.hh"

namespace deadfood::core {

FieldVariant Row::GetField(const std::string& field_name) const {
  if (IsNull(field_name)) {
    return null_t{};
  }
  const size_t offset = schema_.Offset(field_name);
  const auto info = schema_.field_info(field_name);
  switch (info.type()) {
    case Field::FieldType::Bool:
      return storage_.ReadBool(offset);
    case Field::FieldType::Int:
      return storage_.ReadInt(offset);
    case Field::FieldType::Float:
      return storage_.ReadFloat(offset);
    case Field::FieldType::Double:
      return storage_.ReadDouble(offset);
    case Field::FieldType::Varchar:
      return storage_.ReadVarchar(offset, info.size());
  }
}

void Row::SetField(const std::string& field_name, const FieldVariant& value) {
  if (std::holds_alternative<null_t>(value)) {
    const size_t idx = schema_.Index(field_name);
    const size_t raw_idx = idx / 8;
    const auto bit_mask = static_cast<uint8_t>(1 << (idx % 8));
    const uint8_t modified_byte =
        static_cast<uint8_t>(storage_.ReadByte(raw_idx)) | bit_mask;
    storage_.WriteByte(raw_idx, static_cast<char>(modified_byte));
    return;
  }
  const size_t offset = schema_.Offset(field_name);
  const auto info = schema_.field_info(field_name);
  std::visit(
      [&](auto&& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, bool>) {
          if (info.type() == Field::FieldType::Bool) {
            storage_.WriteBool(offset, v);
          }
        } else if constexpr (std::is_same_v<T, int>) {
          if (info.type() == Field::FieldType::Int) {
            storage_.WriteInt(offset, v);
          }
        } else if constexpr (std::is_same_v<T, float>) {
          if (info.type() == Field::FieldType::Float) {
            storage_.WriteFloat(offset, v);
          }
        } else if constexpr (std::is_same_v<T, double>) {
          if (info.type() == Field::FieldType::Double) {
            storage_.WriteDouble(offset, v);
          }
        } else if constexpr (std::is_same_v<T, std::span<char>>) {
          if (info.type() == Field::FieldType::Varchar) {
            storage_.WriteVarchar(offset, v.data(), info.size());
          }
        }
      },
      value);
}

bool Row::IsNull(const std::string& field_name) const {
  const size_t idx = schema_.Index(field_name);
  const size_t raw_idx = idx / 8;
  const auto bit_mask = static_cast<uint8_t>(1 << (idx % 8));
  return static_cast<uint8_t>(storage_.ReadByte(raw_idx)) & bit_mask;
}

}  // namespace deadfood::core