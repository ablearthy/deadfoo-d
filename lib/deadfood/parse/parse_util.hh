#pragma once

#include <iterator>
#include <string>

#include <deadfood/util/parse.hh>
#include <deadfood/util/str.hh>

#include <deadfood/lex/lex_util.hh>

namespace deadfood::parse::util {

template <std::forward_iterator It>
inline std::string ParseIdWithoutDot(It& it, const It end) {
  auto id = ExpectIdentifier(it, end);
  RaiseParserErrorIf(deadfood::util::ContainsDot(id),
                     "invalid identifier: contains dot");
  ++it;
  return id;
}

template <std::forward_iterator It>
inline std::pair<std::optional<std::string>, std::string> ParseFullFieldName(
    It& it, const It end) {
  auto id = ExpectIdentifier(it, end);
  ++it;
  return deadfood::parse::util::GetFullFieldName(id);
}

template <std::forward_iterator It>
inline void ParseKeyword(It& it, const It end, lex::Keyword keyword) {
  if (it != end && lex::IsKeyword(*it, keyword)) {
    ++it;
  } else {
    throw ParserError("expected keyword `" +
                      lex::util::GetStringByKeyword(keyword) + "`");
  }
}

template <std::forward_iterator It>
inline void ParseSymbol(It& it, const It end, lex::Symbol symbol) {
  if (it != end && lex::IsSymbol(*it, symbol)) {
    ++it;
  } else {
    std::string ret = "expected symbol `";
    ret += lex::util::GetCharBySymbol(symbol);
    ret += '`';
    throw ParserError(ret);
  }
}

}  // namespace deadfood::parse::util