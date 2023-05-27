#pragma once

#include <algorithm>
#include <string>
#include <optional>

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

inline std::optional<std::pair<std::string, std::string>> SplitOnDot(
    const std::string& s) {
  auto pos = std::find(s.cbegin(), s.cend(), '.');
  if (pos == s.end()) {
    return std::nullopt;
  }
  std::string before(s.begin(), pos);
  std::string after(pos + 1, s.end());
  return std::make_optional<std::pair<std::string, std::string>>(before, after);
}

}  // namespace deadfood::util