#include "delete_parser.hh"

#include <deadfood/parse/parse_util.hh>
#include <deadfood/parse/expr_tree_parser.hh>

namespace deadfood::parse {

template <std::forward_iterator It>
  requires std::same_as<std::iter_value_t<It>, lex::Token>
query::DeleteQuery ParseDeleteQueryInternal(It& it, const It end) {
  util::ParseKeyword(it, end, lex::Keyword::Delete);
  util::ParseKeyword(it, end, lex::Keyword::From);
  auto table_name = util::ParseTableName(it, end);
  if (it == end || !lex::IsKeyword(*it, lex::Keyword::Where)) {
    return {table_name, std::nullopt};
  }
  util::ParseKeyword(it, end, lex::Keyword::Where);
  auto predicate = ParseExprTree(it, end);
  return {table_name, predicate};
}

query::DeleteQuery ParseDeleteQuery(const std::vector<lex::Token>& tokens) {
  auto it = tokens.begin();
  auto ret = ParseDeleteQueryInternal(it, tokens.end());

  util::RaiseParserErrorIf(it != tokens.end(), "unexpected end");
  return ret;
}

}  // namespace deadfood::parse