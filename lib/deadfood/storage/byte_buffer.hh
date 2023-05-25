#pragma once

#include <memory>
#include <span>

namespace deadfood::storage {

class ByteBuffer {
 public:
  explicit ByteBuffer(size_t size)
      : size_{size}, storage_{std::make_unique<char[]>(size)} {}

  ByteBuffer(size_t size, std::unique_ptr<char[]> ptr);

  [[nodiscard]] const char* data() const;
  [[nodiscard]] size_t size() const;

  [[nodiscard]] char ReadByte(size_t idx) const;
  [[nodiscard]] bool ReadBool(size_t offset) const;
  [[nodiscard]] int ReadInt(size_t offset) const;
  [[nodiscard]] float ReadFloat(size_t offset) const;
  [[nodiscard]] double ReadDouble(size_t offset) const;
  [[nodiscard]] std::string ReadVarchar(size_t offset, size_t count) const;

  void WriteByte(size_t offset, char value);
  void WriteBool(size_t offset, bool value);
  void WriteInt(size_t offset, int value);
  void WriteFloat(size_t offset, float value);
  void WriteDouble(size_t offset, double value);
  void WriteVarchar(size_t offset, const char* value, size_t size);

 private:
  size_t size_;
  std::unique_ptr<char[]> storage_;
};

}  // namespace deadfood::storage
