#pragma once

#include <deadfood/lex/lex.hh>
#include <deadfood/parse/parser_error.hh>

namespace deadfood::parse::util {
template <typename It>
inline void ExpectKeyword(const It& it, const It end,
                          const lex::Keyword& keyword,
                          const std::string& message) {
  if (it == end || !lex::IsKeyword(*it, keyword)) {
    throw ParserError(message);
  }
}

template <typename It>
inline void ExpectSymbol(const It& it, const It end, const lex::Symbol& symbol,
                         const std::string& message) {
  if (it == end || !lex::IsSymbol(*it, symbol)) {
    throw ParserError(message);
  }
}

template <typename It>
inline void ExpectNotEnd(const It& it, const It end) {
  if (it == end) {
    throw ParserError("expected token");
  }
}

template <typename It>
inline void ExpectNotEnd(const It& it, const It end,
                         const std::string& message) {
  if (it == end) {
    throw ParserError(message);
  }
}

inline void RaiseParserErrorIf(bool b, const std::string& message) {
  if (b) {
    throw ParserError(message);
  }
}

}  // namespace deadfood::parse::util