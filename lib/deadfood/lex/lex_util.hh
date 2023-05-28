#pragma once

#include <deadfood/lex/lex.hh>

namespace deadfood::lex::util {

char GetCharBySymbol(Symbol sym);

std::string GetStringByKeyword(Keyword key);

}  // namespace deadfood::lex::util