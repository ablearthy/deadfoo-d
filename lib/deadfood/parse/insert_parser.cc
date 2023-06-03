#include "insert_parser.hh"

#include <deadfood/parse/expr_tree_parser.hh>

#include <deadfood/parse/parser_error.hh>

#include <deadfood/util/parse.hh>
#include <deadfood/util/str.hh>
#include <deadfood/parse/parse_util.hh>

namespace deadfood::parse {

template <std::forward_iterator It>
  requires std::is_same_v<std::iter_value_t<It>, lex::Token>
query::InsertQuery ParseInsertQueryInternal(It& it, const It end) {
  util::ParseKeyword(it, end, lex::Keyword::Insert);
  util::ParseKeyword(it, end, lex::Keyword::Into);
  const auto table_name = util::ParseIdWithoutDot(it, end);

  std::optional<std::vector<std::string>> field_names = std::nullopt;
  if (lex::IsSymbol(*it, lex::Symbol::LParen)) {
    ++it;
    auto field_name = util::ParseIdWithoutDot(it, end);
    field_names = {field_name};
    while (lex::IsSymbol(*it, lex::Symbol::Comma)) {
      ++it;  // comma
      field_name = util::ParseIdWithoutDot(it, end);
      field_names->emplace_back(field_name);
    }
    util::ParseSymbol(it, end, lex::Symbol::RParen);
  }

  util::ParseKeyword(it, end, lex::Keyword::Values);
  std::vector<std::vector<expr::FactorTree>> values;
  while (true) {
    std::vector<expr::FactorTree> row;
    util::ParseSymbol(it, end, lex::Symbol::LParen);

    while (true) {
      auto expr = ParseExprTree(it, end);
      row.emplace_back(std::move(expr));

      if (it != end && lex::IsSymbol(*it, lex::Symbol::Comma)) {
        util::ParseSymbol(it, end, lex::Symbol::Comma);
      } else {
        break;
      }
    }
    values.emplace_back(std::move(row));
    util::ParseSymbol(it, end, lex::Symbol::RParen);

    if (it != end && lex::IsSymbol(*it, lex::Symbol::Comma)) {
      util::ParseSymbol(it, end, lex::Symbol::Comma);
    } else {
      break;
    }
  }

  return query::InsertQuery{
      .table_name = table_name, .fields = field_names, .values = values};
}

query::InsertQuery ParseInsertQuery(const std::vector<lex::Token>& tokens) {
  auto it = tokens.begin();
  const auto ret = ParseInsertQueryInternal(it, tokens.end());
  util::RaiseParserErrorIf(it != tokens.end(), "unexpected end");
  return ret;
}

}  // namespace deadfood::parse