#include "create_table_parser.hh"

#include <algorithm>
#include <optional>

#include <deadfood/core/field.hh>
#include <deadfood/parse/parser_error.hh>
#include <deadfood/util/parse.hh>
#include <deadfood/util/str.hh>

#include <deadfood/parse/parse_util.hh>

namespace deadfood::parse {

struct Field {
  std::string name;
  core::Field type;
  bool is_unique;
  bool may_be_null;
};

struct Unique {};
struct NotNull {};
struct PrimaryKey {};

using LocalConstraint = std::variant<Unique, NotNull, PrimaryKey>;

template <typename It>
core::Field ParseFieldType(It& it, const It end) {
  if (it == end) {
    throw ParserError("expected type of field");
  }
  if (lex::IsKeyword(*it, lex::Keyword::Int)) {
    ++it;
    return core::field::kIntField;
  }
  if (lex::IsKeyword(*it, lex::Keyword::Boolean)) {
    ++it;
    return core::field::kBoolField;
  }
  if (lex::IsKeyword(*it, lex::Keyword::Float)) {
    ++it;
    return core::field::kFloatField;
  }
  if (lex::IsKeyword(*it, lex::Keyword::Double)) {
    ++it;
    return core::field::kDoubleField;
  }

  util::ParseKeyword(it, end, lex::Keyword::Varchar);
  util::ParseSymbol(it, end, lex::Symbol::LParen);
  auto size = util::ParseInt(it, end);
  util::ParseSymbol(it, end, lex::Symbol::RParen);
  return core::field::VarcharField(static_cast<size_t>(size));
}

template <typename It>
std::optional<LocalConstraint> TryParseLocalConstraint(It& it, const It end) {
  if (lex::IsKeyword(*it, lex::Keyword::Primary)) {
    ++it;
    util::ParseKeyword(it, end, lex::Keyword::Key);
    return PrimaryKey{};
  }

  if (lex::IsKeyword(*it, lex::Keyword::Not)) {
    ++it;
    util::ParseKeyword(it, end, lex::Keyword::Null);
    return NotNull{};
  }

  if (lex::IsKeyword(*it, lex::Keyword::Unique)) {
    ++it;
    return Unique{};
  }
  return std::nullopt;
}

template <std::forward_iterator It>
Field ParseField(It& it, const It end) {
  auto field_name = util::ParseIdWithoutDot(it, end);
  const core::Field field_type = ParseFieldType(it, end);

  bool is_unique = false;
  bool may_be_null = true;

  if (it == end) {
    return {field_name, field_type, is_unique, may_be_null};
  }

  while (it != end) {
    if (lex::IsSymbol(*it, lex::Symbol::RParen) ||
        lex::IsSymbol(*it, lex::Symbol::Comma)) {
      break;
    }
    if (const auto local_constraint = TryParseLocalConstraint(it, end)) {
      if (std::holds_alternative<NotNull>(*local_constraint)) {
        may_be_null = false;
      } else if (std::holds_alternative<Unique>(*local_constraint)) {
        is_unique = true;
      } else if (std::holds_alternative<PrimaryKey>(*local_constraint)) {
        may_be_null = false;
        is_unique = true;
      }
    } else {
      break;
    }
  }
  return {field_name, field_type, is_unique, may_be_null};
}

template <typename It>
core::ReferencesConstraint ParseReferencesConstraint(
    const std::string& table_name, It& it, const It end) {
  util::ParseKeyword(it, end, lex::Keyword::Foreign);
  util::ParseKeyword(it, end, lex::Keyword::Key);
  const auto slave_field_name = util::ParseIdWithoutDot(it, end);
  util::ParseKeyword(it, end, lex::Keyword::References);
  const auto master_table_name = util::ParseIdWithoutDot(it, end);
  util::ParseSymbol(it, end, lex::Symbol::LParen);
  const auto master_field_name = util::ParseIdWithoutDot(it, end);
  util::ParseSymbol(it, end, lex::Symbol::RParen);

  return core::ReferencesConstraint{
      .master_table = master_table_name,
      .master_field = master_field_name,
      .slave_table = table_name,
      .slave_field = slave_field_name,
      .on_delete = core::ReferencesConstraint::OnAction::NoAction,
      .on_update = core::ReferencesConstraint::OnAction::NoAction};
}

std::pair<query::CreateTableQuery, std::vector<core::Constraint>>
ParseCreateTableQuery(const std::vector<lex::Token>& tokens) {
  auto it = tokens.cbegin();
  const auto end = tokens.cend();

  util::ParseKeyword(it, end, lex::Keyword::Create);
  util::ParseKeyword(it, end, lex::Keyword::Table);
  const auto table_name = util::ParseIdWithoutDot(it, end);
  util::ParseSymbol(it, end, lex::Symbol::LParen);

  query::CreateTableQuery query(table_name);
  std::vector<core::Constraint> constraints;

  while (!lex::IsSymbol(*it, lex::Symbol::RParen)) {
    if (lex::IsKeyword(*it, lex::Keyword::Foreign)) {
      auto constraint =
          ParseReferencesConstraint(table_name, it, tokens.cend());
      constraints.emplace_back(std::move(constraint));
    } else {
      const auto field = ParseField(it, tokens.cend());
      if (query.Contains(field.name)) {
        throw ParserError("duplicate field");
      }
      query.AddField(field.name, field.type, field.is_unique,
                     field.may_be_null);
      if (lex::IsSymbol(*it, lex::Symbol::Comma)) {
        ++it;
      }
    }
  }
  return {query, constraints};
}

}  // namespace deadfood::parse
