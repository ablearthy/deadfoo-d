#include "insert_parser.hh"

#include <deadfood/parse/parser_error.hh>

#include <deadfood/util/parse.hh>
#include <deadfood/util/str.hh>

namespace deadfood::parse {

query::InsertQuery ParseInsertQuery(const std::vector<lex::Token>& tokens) {
  if (tokens.size() <= 4 || !lex::IsKeyword(tokens[0], lex::Keyword::Insert) ||
      !lex::IsKeyword(tokens[1], lex::Keyword::Into)) {
    throw ParserError("invalid query");
  }
  if (!std::holds_alternative<lex::Identifier>(tokens[2])) {
    throw ParserError("expected table table name");
  }
  auto table_name = std::get<lex::Identifier>(tokens[2]).id;
  if (deadfood::util::ContainsDot(table_name)) {
    throw ParserError("invalid name of table");
  }
  auto it = tokens.begin() + 3;
  std::optional<std::vector<std::string>> field_names = std::nullopt;
  if (lex::IsSymbol(*it, lex::Symbol::LParen)) {
    field_names = std::vector<std::string>{};
    ++it;
    while (!lex::IsSymbol(*it, lex::Symbol::RParen)) {
      ++it;
      if (!std::holds_alternative<lex::Identifier>(*it)) {
        throw ParserError("expected field name");
      }
      auto field_name = std::get<lex::Identifier>(*it).id;
      field_names->emplace_back(field_name);
      if (deadfood::util::ContainsDot(field_name)) {
        throw ParserError("invalid name of field");
      }
      ++it;
      if (it == tokens.end()) {
        throw ParserError("expected `)`");
      }
      if (lex::IsSymbol(*it, lex::Symbol::Comma)) {
        ++it;
      }
      if (it == tokens.end()) {
        throw ParserError("expected `)`");
      }
    }
    ++it;
  }
  // TODO: VALUES, and parser expressions
}

}  // namespace deadfood::parse