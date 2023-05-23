#include "byte_buffer.hh"

namespace deadfood::storage {

const char* ByteBuffer::data() const { return storage_.get(); }

size_t ByteBuffer::size() const { return size_; }

char ByteBuffer::ReadByte(size_t idx) const { return storage_[idx]; }

bool ByteBuffer::ReadBool(size_t offset) const { return ReadInt(offset) != 0; }

int ByteBuffer::ReadInt(size_t offset) const {
  return static_cast<int>(static_cast<uint8_t>(storage_[offset])) |
         (static_cast<int>(static_cast<uint8_t>(storage_[offset + 1])) << 8) |
         (static_cast<int>(static_cast<uint8_t>(storage_[offset + 2])) << 16) |
         (static_cast<int>(static_cast<uint8_t>(storage_[offset + 3])) << 24);
}

float ByteBuffer::ReadFloat(size_t offset) const {
  constexpr size_t kFloatSize = 4;
  union {
    float d;
    char s[kFloatSize];
  } tmp{};

  std::copy(storage_.get() + offset, storage_.get() + offset + kFloatSize,
            tmp.s);
  // TODO: check endianness
  return tmp.d;
}

double ByteBuffer::ReadDouble(size_t offset) const {
  constexpr size_t kDoubleSize = 8;
  union {
    double d;
    char s[kDoubleSize];
  } tmp{};

  std::copy(storage_.get() + offset, storage_.get() + offset + kDoubleSize,
            tmp.s);
  // TODO: check endianness
  return tmp.d;
}

std::span<char> ByteBuffer::ReadVarchar(size_t offset, size_t count) const {
  return {storage_.get() + offset, count};
}

void ByteBuffer::WriteBool(size_t offset, bool value) {
  WriteInt(offset, value);
}

void ByteBuffer::WriteInt(size_t offset, int value) {
  storage_[offset] = static_cast<char>(value & 0xff);
  storage_[offset + 1] = static_cast<char>((value >> 8) & 0xff);
  storage_[offset + 2] = static_cast<char>((value >> 16) & 0xff);
  storage_[offset + 3] = static_cast<char>((value >> 24) & 0xff);
}

void ByteBuffer::WriteFloat(size_t offset, float value) {
  constexpr size_t kFloatSize = 4;
  union {
    float d;
    char s[kFloatSize];
  } tmp{.d = value};

  std::copy(tmp.s, tmp.s + kFloatSize, storage_.get() + offset);
  // TODO: check endianness
}

void ByteBuffer::WriteDouble(size_t offset, double value) {
  constexpr size_t kDoubleSize = 8;
  union {
    double d;
    char s[kDoubleSize];
  } tmp{.d = value};

  std::copy(tmp.s, tmp.s + kDoubleSize, storage_.get() + offset);
  // TODO: check endianness
}

void ByteBuffer::WriteVarchar(size_t offset, const char* value, size_t size) {
  std::copy(value, value + size, storage_.get() + offset);
}

void ByteBuffer::WriteByte(size_t offset, char value) {
  storage_[offset] = value;
}

}  // namespace deadfood::storage