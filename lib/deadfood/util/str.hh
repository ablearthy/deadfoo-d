#pragma once

#include <algorithm>
#include <string>

namespace deadfood::util {

inline bool ContainsDot(const std::string& str) {
  return std::find(str.cbegin(), str.cend(), '.') != str.cend();
}

template <typename T>
inline std::basic_string<T> lowercase(const std::basic_string<T>& s) {
  std::basic_string<T> s2 = s;
  std::transform(s2.begin(), s2.end(), s2.begin(), [](const T v) {
    return static_cast<T>(std::tolower(static_cast<unsigned char>(v)));
  });
  return s2;
}

}  // namespace deadfood::util