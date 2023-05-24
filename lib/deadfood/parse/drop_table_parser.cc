#include "drop_table_parser.hh"

#include <algorithm>
#include <deadfood/parse/parser_error.hh>

#include <deadfood/util/str.hh>

namespace deadfood::parse {

std::string ParseDropTableQuery(const std::vector<lex::Token>& tokens) {
  if (tokens.size() != 3) {
    throw ParserError("unexpected input");
  }
  if (!lex::IsKeyword(tokens[0], lex::Keyword::Drop)) {
    throw ParserError("expected `DROP`");
  }
  if (!lex::IsKeyword(tokens[1], lex::Keyword::Table)) {
    throw ParserError("expected `TABLE`");
  }
  if (!std::holds_alternative<lex::Identifier>(tokens[2])) {
    throw ParserError("expected identifier: table name");
  }
  auto table_name = std::get<lex::Identifier>(tokens[2]).id;
  if (deadfood::util::ContainsDot(table_name)) {
    throw ParserError("table name contains invalid characters");
  }
  return table_name;
}

}  // namespace deadfood::parse