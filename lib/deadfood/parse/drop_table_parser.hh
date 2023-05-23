#pragma once

#include <stdexcept>
#include <deadfood/lex/lex.hh>

namespace deadfood::parse {

std::string ParseDropTableQuery(const std::vector<lex::Token>& tokens);

}  // namespace deadfood::parse
