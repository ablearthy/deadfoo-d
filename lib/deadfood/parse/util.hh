#pragma once

#include <iterator>
#include <string>

#include <deadfood/util/parse.hh>
#include <deadfood/util/str.hh>

namespace deadfood::parse::util {

template <std::forward_iterator It>
inline std::string ParseTableName(It& it, const It end) {
  auto id = ExpectIdentifier(it, end);
  RaiseParserErrorIf(deadfood::util::ContainsDot(id),
                     "table name contains dot");
  ++it;
  return id;
}

}  // namespace deadfood::parse::util