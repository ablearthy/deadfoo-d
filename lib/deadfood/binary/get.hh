#pragma once

#include <istream>
#include <cstdint>
#include <type_traits>
#include <concepts>

namespace deadfood::binary {

template <std::unsigned_integral T>
T GetUint(std::istream& stream) {
  const size_t bytes_count = sizeof(T);
  T num = 0;
  for (size_t i = 0; i < bytes_count; ++i) {
    char ch;
    stream.get(ch);
    num = (num << 8) | static_cast<T>(static_cast<uint8_t>(ch));
  }
  return num;
};

template <std::signed_integral T>
T GetInt(std::istream& stream) {
  using UnsignedT = std::make_unsigned<T>::value;
  return static_cast<T>(GetUint<UnsignedT>(stream));
}

std::string GetCString(std::istream& stream) {
  char ch;
  std::string ret;
  while (stream) {
    stream.get(ch);
    if (ch == 0) {
      break;
    }
    ret += ch;
  }
  return ret;
}

template <std::floating_point T>
T GetFloatingPoint(std::istream& stream) {
  union {
    char buf[sizeof(T)];
    T val;
  } un;
  stream.read(un.buf, sizeof(T));
  return un.val;
}

}  // namespace deadfood::binary