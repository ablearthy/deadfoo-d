#pragma once

#include <algorithm>
#include <string>

namespace deadfood::util {

bool ContainsDot(const std::string& str) {
  return std::find(str.cbegin(), str.cend(), '.') != str.cend();
}

}  // namespace deadfood::util