#include "drop_table_parser.hh"

#include <algorithm>
#include <deadfood/parse/parser_error.hh>

#include <deadfood/parse/parse_util.hh>

namespace deadfood::parse {

std::string ParseDropTableQuery(const std::vector<lex::Token>& tokens) {
  auto it = tokens.begin();
  const auto end = tokens.end();
  util::ParseKeyword(it, end, lex::Keyword::Drop);
  util::ParseKeyword(it, end, lex::Keyword::Table);
  auto table_name = util::ParseIdWithoutDot(it, end);
  util::RaiseParserErrorIf(it != end, "unexpected end");
  return table_name;
}

}  // namespace deadfood::parse