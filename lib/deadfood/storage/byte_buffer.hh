#pragma once

#include <memory>
#include <span>

namespace deadfood::storage {

class ByteBuffer {
 public:
  explicit ByteBuffer(size_t size)
      : size_{size}, storage_{std::make_unique<char[]>(size)} {}

  char ReadByte(size_t idx) const;
  bool ReadBool(size_t offset) const;
  int ReadInt(size_t offset) const;
  float ReadFloat(size_t offset) const;
  double ReadDouble(size_t offset) const;
  std::span<char> ReadVarchar(size_t offset, size_t count) const;

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
