#include "update_parser.hh"

#include <deadfood/util/parse.hh>
#include <deadfood/parse/parse_util.hh>
#include <deadfood/parse/expr_tree_parser.hh>

namespace deadfood::parse {

template <std::forward_iterator It>
  requires std::same_as<std::iter_value_t<It>, lex::Token>
query::UpdateQuery ParseUpdateQueryInternal(It& it, const It end) {
  query::UpdateQuery ret;
  util::ParseKeyword(it, end, lex::Keyword::Update);
  ret.table_name = util::ParseIdWithoutDot(it, end);
  util::ParseKeyword(it, end, lex::Keyword::Set);

  auto lhs = util::ParseIdWithoutDot(it, end);
  util::ParseSymbol(it, end, lex::Symbol::Eq);
  auto rhs = ParseExprTree(it, end);
  ret.sets.emplace(lhs, rhs);

  while (it != end && lex::IsSymbol(*it, lex::Symbol::Comma)) {
    ++it;
    lhs = util::ParseIdWithoutDot(it, end);
    util::ParseSymbol(it, end, lex::Symbol::Eq);
    rhs = ParseExprTree(it, end);
    ret.sets.emplace(lhs, rhs);
  }
  if (it == end || !lex::IsKeyword(*it, lex::Keyword::Where)) {
    return ret;
  }
  ++it;
  ret.predicate = ParseExprTree(it, end);
  return ret;
}

query::UpdateQuery ParseUpdateQuery(const std::vector<lex::Token>& tokens) {
  auto it = tokens.begin();
  auto ret = ParseUpdateQueryInternal(it, tokens.end());

  util::RaiseParserErrorIf(it != tokens.end(), "unexpected end");
  return ret;
}

}  // namespace deadfood::parse