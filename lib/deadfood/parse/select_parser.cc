#include "select_parser.hh"

namespace deadfood::parse {

query::SelectQuery ParseSelectQuery(const std::vector<lex::Token>& tokens) {
  auto it = tokens.begin();
  return ParseSelectQuery(it, tokens.end());
}

}  // namespace deadfood::parse