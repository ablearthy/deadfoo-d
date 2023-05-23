#include "drop_table_parser.hh"

#include <algorithm>

namespace deadfood::parse {

bool ContainsDot(const std::string& str) {
  return std::find(str.cbegin(), str.cend(), '.') != str.cend();
}

std::string ParseDropTableQuery(const std::vector<lex::Token>& tokens) {
  if (tokens.size() != 3) {
    throw DropTableParseError("unexpected input");
  }
  if (!lex::IsKeyword(tokens[0], lex::Keyword::Drop)) {
    throw DropTableParseError("expected `DROP`");
  }
  if (!lex::IsKeyword(tokens[1], lex::Keyword::Table)) {
    throw DropTableParseError("expected `TABLE`");
  }
  if (!std::holds_alternative<lex::Identifier>(tokens[2])) {
    throw DropTableParseError("expected identifier: table name");
  }
  auto table_name = std::get<lex::Identifier>(tokens[2]).id;
  if (ContainsDot(table_name)) {
    throw DropTableParseError("table name contains invalid characters");
  }
  return table_name;
}
}  // namespace deadfood::parse