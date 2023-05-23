#pragma once

#include <stdexcept>
#include <deadfood/lex/lex.hh>

namespace deadfood::parse {

class DropTableParseError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

std::string ParseCreateTableQuery(const std::vector<lex::Token>& tokens);

}  // namespace deadfood::parse
