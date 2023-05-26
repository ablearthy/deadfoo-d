#pragma once

#include <deadfood/parse/select_parser_fwd.hh>
#include <deadfood/lex/lex.hh>
#include <deadfood/util/parse.hh>
#include <deadfood/parse/expr_tree_parser.hh>
#include <deadfood/parse/parser_error.hh>
#include <deadfood/util/str.hh>

namespace deadfood::parse {

namespace {

template <std::forward_iterator It>
std::optional<query::JoinType> ParseJoinType(It& it, const It& end) {
  if (it == end) {
    return std::nullopt;
  }
  if (lex::IsKeyword(*it, lex::Keyword::Join)) {
    ++it;
    return query::JoinType::Inner;
  }

  if (const auto* id = std::get_if<lex::Identifier>(&*it)) {
    const auto lc_str = deadfood::util::lowercase(id->id);
    if ((lc_str == "left" || lc_str == "inner" || lc_str == "right") &&
        it + 1 != end && lex::IsKeyword(*(it + 1), lex::Keyword::Join)) {
      it += 2;
      if (lc_str == "left") {
        return query::JoinType::Left;
      } else if (lc_str == "inner") {
        return query::JoinType::Inner;
      } else if (lc_str == "right") {
        return query::JoinType::Right;
      }
      return std::nullopt;
    }
  }
  return std::nullopt;
}

template <std::forward_iterator It>
query::Selector ParseSelector(It& it, const It end) {
  if (lex::IsSymbol(*it, lex::Symbol::Mul)) {
    ++it;
    return query::SelectAllSelector{};
  }

  if (const auto* id = std::get_if<lex::Identifier>(&*it)) {
    util::ExpectNotEnd(it + 1, end);
    if (lex::IsSymbol(*(it + 1), lex::Symbol::Comma) ||
        lex::IsKeyword(*(it + 1), lex::Keyword::From)) {
      ++it;
      return id->id;
    }
  }
  auto expr = ParseExprTree(it, end);
  util::ExpectKeyword(it, end, lex::Keyword::As, "expression must have name");
  ++it;
  util::ExpectNotEnd(it, end);
  util::RaiseParserErrorIf(!std::holds_alternative<lex::Identifier>(*it),
                           "expected name");
  auto field_name = std::get<lex::Identifier>(*it).id;
  ++it;
  return query::FieldSelector{.expr = std::move(expr),
                              .field_name = std::move(field_name)};
}

}  // namespace

template <std::forward_iterator It>
inline query::SelectQuery ParseSelectQuery(It& it, const It end) {
  query::SelectQuery ret;
  util::ExpectKeyword(it, end, lex::Keyword::Select, "expected `SELECT`");
  ++it;
  while (it != end || !(lex::IsKeyword(*it, lex::Keyword::From) ||
                        lex::IsKeyword(*it, lex::Keyword::Where))) {
    auto selector = ParseSelector(it, end);
    ret.selectors.emplace_back(std::move(selector));
    if (it != end && lex::IsSymbol(*it, lex::Symbol::Comma)) {
      ++it;
    }
  }

  if (ret.selectors.empty()) {
    throw ParserError("expected some selectors");
  }

  if (it == end) {
    return ret;
  }

  if (lex::IsKeyword(*it, lex::Keyword::From)) {
    ++it;
    while (it != end || !lex::IsKeyword(*it, lex::Keyword::Where)) {
      {
        std::forward_iterator auto copy_it = it;
        if (ParseJoinType(copy_it, end).has_value()) {
          break;
        }
      }
      // TODO: parse sources
    }

    if (it == end) {
      return ret;
    }

    if (auto join_type = ParseJoinType(it, end)) {
      // TODO: parse joins
    }
  }

  if (it == end) {
    return ret;
  }

  if (lex::IsKeyword(*it, lex::Keyword::Where)) {
    ++it;
    auto predicate = ParseExprTree(it, end);
    if (it != end) {
      throw ParserError("not expected tokens after WHERE predicate");
    }
    ret.predicate = std::move(predicate);
    return ret;
  }

  return ret;
}

query::SelectQuery ParseSelectQuery(const std::vector<lex::Token>& tokens);

}  // namespace deadfood::parse