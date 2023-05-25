#include "insert_parser.hh"

#include <deadfood/parse/expr_tree_parser.hh>

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
  util::RaiseParserErrorIf(deadfood::util::ContainsDot(table_name),
                           "invalid name of table");
  auto it = tokens.begin() + 3;
  std::optional<std::vector<std::string>> field_names = std::nullopt;
  if (lex::IsSymbol(*it, lex::Symbol::LParen)) {
    field_names = std::vector<std::string>{};
    ++it;
    while (!lex::IsSymbol(*it, lex::Symbol::RParen)) {
      util::RaiseParserErrorIf(!std::holds_alternative<lex::Identifier>(*it),
                               "expected field name");

      auto field_name = std::get<lex::Identifier>(*it).id;
      field_names->emplace_back(field_name);
      util::RaiseParserErrorIf(deadfood::util::ContainsDot(field_name),
                               "invalid name of field");
      ++it;
      util::ExpectNotEnd(it, tokens.end(), "expected `)`");
      if (lex::IsSymbol(*it, lex::Symbol::Comma)) {
        ++it;
      }
      util::ExpectNotEnd(it, tokens.end(), "expected `)`");
    }
    ++it;
  }

  if (it == tokens.end() || !lex::IsKeyword(*it, lex::Keyword::Values)) {
    throw ParserError("expected `VALUES`");
  }
  ++it;
  std::vector<std::vector<expr::FactorTree>> values;
  while (it != tokens.end()) {
    util::ExpectSymbol(it, tokens.end(), lex::Symbol::LParen, "expected `(`");
    ++it;
    util::ExpectNotEnd(it, tokens.end());
    std::vector<expr::FactorTree> row;
    while (!lex::IsSymbol(*it, lex::Symbol::RParen)) {
      auto expr = ParseExprTree(it, tokens.end());
      row.emplace_back(std::move(expr));
      util::ExpectNotEnd(it, tokens.end());
      if (lex::IsSymbol(*it, lex::Symbol::Comma)) {
        ++it;
      } else if (!lex::IsSymbol(*it, lex::Symbol::RParen)) {
        throw ParserError("invalid expression");
      }
    }
    values.emplace_back(std::move(row));
    ++it;
    if (it == tokens.end()) {
      break;
    } else {
      util::ExpectSymbol(it, tokens.end(), lex::Symbol::Comma, "expected `,`");
      ++it;
    }
  }
  return query::InsertQuery{
      .table_name = table_name, .fields = field_names, .values = values};
}

}  // namespace deadfood::parse