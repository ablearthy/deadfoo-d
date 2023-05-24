#pragma once

#include <deadfood/lex/lex.hh>
#include <deadfood/parse/parser_error.hh>

namespace deadfood::parse::util {
template <typename It>
void ExpectKeyword(const It& it, const It end, const lex::Keyword& keyword,
                   const std::string& message) {
  if (it == end || !lex::IsKeyword(*it, keyword)) {
    throw ParserError(message);
  }
}

template <typename It>
void ExpectSymbol(const It& it, const It end, const lex::Symbol& symbol,
                  const std::string& message) {
  if (it == end || !lex::IsSymbol(*it, symbol)) {
    throw ParserError(message);
  }
}
}  // namespace deadfood::parse::util