#pragma once

#include <ostream>
#include <cstdint>
#include <type_traits>
#include <concepts>

namespace deadfood::binary {

template <std::unsigned_integral T>
void PutUint(std::ostream& stream, T val) {
  const size_t bytes_count = sizeof(T);
  for (int i = bytes_count - 1; i >= 0; --i) {
    auto shift = static_cast<size_t>(8 * i);
    const T mask = 0xff << shift;
    const auto byte = static_cast<uint8_t>((val & mask) >> shift);
    stream.put(static_cast<char>(byte));
  }
}

template <std::signed_integral T>
void PutInt(std::ostream& stream, T val) {
  PutUint(stream, std::make_unsigned_t<T>(val));
}

void PutCString(std::ostream& stream, const std::string& string) {
  stream.write(string.c_str(), static_cast<long>(string.size() + 1));
}

void PutBytes(std::ostream& stream, const char* bytes, size_t size) {
  stream.write(bytes, static_cast<long>(size));
}

template <std::floating_point T>
void PutFloatingPoint(std::ostream& stream, T val) {
  union {
    char buf[sizeof(T)];
    T val;
  } un;
  un.val = val;
  stream.write(un.buf, sizeof(T));
}

}  // namespace deadfood::binary