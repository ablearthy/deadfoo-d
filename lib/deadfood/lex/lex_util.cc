#include "lex_util.hh"

namespace deadfood::lex::util {

char GetCharBySymbol(Symbol sym) {
  for (const auto& [ch, s] : kCharToSymbol) {
    if (s == sym) {
      return ch;
    }
  }
}
std::string GetStringByKeyword(Keyword key) {
  for (const auto& [s, k] : kKeywordLiteralToKeyword) {
    if (k == key) {
      return s;
    }
  }
}

}  // namespace deadfood::lex::util