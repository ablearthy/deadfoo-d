#include "create_table_parser.hh"

#include <algorithm>
#include <optional>

#include <deadfood/core/field.hh>

namespace deadfood::parse {

bool ContainsDot(const std::string& str) {
  return std::find(str.cbegin(), str.cend(), '.') != str.cend();
}

template <typename It>
void ExpectKeyword(const It& it, const It end, const lex::Keyword& keyword,
                   const std::string& message) {
  if (it == end || !lex::IsKeyword(*it, keyword)) {
    throw CreateTableParseError(message);
  }
}

template <typename It>
void ExpectSymbol(const It& it, const It end, const lex::Symbol& symbol,
                  const std::string& message) {
  if (it == end || !lex::IsSymbol(*it, symbol)) {
    throw CreateTableParseError(message);
  }
}

std::string GetTableName(const lex::Token& tok) {
  if (!std::holds_alternative<lex::Identifier>(tok)) {
    throw CreateTableParseError("invalid name for table");
  }
  const auto& [str] = std::get<lex::Identifier>(tok);
  if (str.cend() != std::find(str.cbegin(), str.cend(), '.')) {
    throw CreateTableParseError("incorrect table name: it contains `.`");
  }
  return str;
}

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
core::Field ReadFieldType(It& it, const It end) {
  if (it == end) {
    throw CreateTableParseError("expected type of field");
  }
  if (lex::IsKeyword(*it, lex::Keyword::Int)) {
    return core::field::kIntField;
  }
  if (lex::IsKeyword(*it, lex::Keyword::Boolean)) {
    return core::field::kBoolField;
  }
  if (lex::IsKeyword(*it, lex::Keyword::Float)) {
    return core::field::kFloatField;
  }
  if (lex::IsKeyword(*it, lex::Keyword::Double)) {
    return core::field::kDoubleField;
  }

  ExpectKeyword(it, end, lex::Keyword::Varchar, "unknown field type");
  ++it;
  ExpectSymbol(it, end, lex::Symbol::LParen, "expected `(`");
  ++it;

  if (it == end || !std::holds_alternative<int>(*it)) {
    throw CreateTableParseError("expected constant int");
  }
  const auto size = std::get<int>(*it);
  if (size <= 0) {
    throw CreateTableParseError("varchar cannot have negative size");
  }
  ++it;

  ExpectSymbol(it, end, lex::Symbol::RParen, "expected `)`");
  return core::field::VarcharField(static_cast<size_t>(size));
}

template <typename It>
std::optional<LocalConstraint> TryReadLocalConstraint(It& it, const It end) {
  if (lex::IsKeyword(*it, lex::Keyword::Primary)) {
    ++it;
    ExpectKeyword(it, end, lex::Keyword::Key, "expected `KEY`");
    return PrimaryKey{};
  }

  if (lex::IsKeyword(*it, lex::Keyword::Not)) {
    ++it;
    ExpectKeyword(it, end, lex::Keyword::Null, "expected `NULL`");
    return NotNull{};
  }
  if (lex::IsKeyword(*it, lex::Keyword::Unique)) {
    return Unique{};
  }
  return std::nullopt;
}

template <std::forward_iterator It>
Field ReadField(It& it, const It end) {
  if (it == end || !std::holds_alternative<lex::Identifier>(*it)) {
    throw CreateTableParseError("expected field name");
  }

  const auto& [field_name] = std::get<lex::Identifier>(*it);
  if (ContainsDot(field_name)) {
    throw CreateTableParseError("incorrect field name: it contains `.`");
  }
  ++it;

  const core::Field field_type = ReadFieldType(it, end);
  ++it;

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
    if (const auto local_constraint = TryReadLocalConstraint(it, end)) {
      if (std::holds_alternative<NotNull>(*local_constraint)) {
        may_be_null = false;
      } else if (std::holds_alternative<Unique>(*local_constraint)) {
        is_unique = true;
      } else if (std::holds_alternative<PrimaryKey>(*local_constraint)) {
        may_be_null = false;
        is_unique = true;
      }
      ++it;
    } else {
      break;
    }
  }
  return {field_name, field_type, is_unique, may_be_null};
}

template <typename It>
core::ReferencesConstraint ParseReferencesConstraint(
    const std::string& table_name, It& it, const It end) {
  ExpectKeyword(it, end, lex::Keyword::Foreign,
                "expected `FOREIGN` while parsing constraint");
  ++it;
  ExpectKeyword(it, end, lex::Keyword::Key,
                "expected `KEY` while parsing constraint");
  ++it;

  if (it == end || !std::holds_alternative<lex::Identifier>(*it)) {
    throw CreateTableParseError("expected field name");
  }
  const auto [slave_field_name] = std::get<lex::Identifier>(*it);
  ++it;
  if (ContainsDot(slave_field_name)) {
    throw CreateTableParseError("expected valid slave field name");
  }

  ExpectKeyword(it, end, lex::Keyword::References,
                "expected `REFERENCES` while parsing constraint");
  ++it;

  if (it == end || !std::holds_alternative<lex::Identifier>(*it)) {
    throw CreateTableParseError("expected master table name");
  }
  const auto [master_table_name] = std::get<lex::Identifier>(*it);
  if (ContainsDot(master_table_name)) {
    throw CreateTableParseError("master table name contains dot");
  }
  ++it;

  ExpectSymbol(it, end, lex::Symbol::LParen,
               "expected `(` while parsing constraint");

  ++it;
  if (it == end || !std::holds_alternative<lex::Identifier>(*it)) {
    throw CreateTableParseError("expected master table name");
  }
  const auto [master_field_name] = std::get<lex::Identifier>(*it);
  if (ContainsDot(master_field_name)) {
    throw CreateTableParseError("master field name contains dot");
  }
  ++it;

  ExpectSymbol(it, end, lex::Symbol::RParen,
               "expected `)` while parsing constraint");
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
  ExpectKeyword(it, tokens.cend(), lex::Keyword::Create, "invalid query");
  ++it;
  ExpectKeyword(it, tokens.cend(), lex::Keyword::Table, "invalid query");
  ++it;

  const auto table_name = GetTableName(*it);
  ++it;

  ExpectSymbol(it, tokens.cend(), lex::Symbol::LParen, "expected `(`");
  ++it;

  query::CreateTableQuery query(table_name);
  std::vector<core::Constraint> constraints;

  while (!lex::IsSymbol(*it, lex::Symbol::RParen)) {
    if (lex::IsKeyword(*it, lex::Keyword::Foreign)) {
      // constraints
      auto constraint =
          ParseReferencesConstraint(table_name, it, tokens.cend());
      constraints.emplace_back(std::move(constraint));
      ++it;
    } else {  // field definition
      const auto field = ReadField(it, tokens.cend());
      if (query.Contains(field.name)) {
        throw CreateTableParseError("duplicate field");
      }
      query.AddField(field.name, field.type, field.is_unique,
                     field.may_be_null);
      if (!lex::IsSymbol(*it, lex::Symbol::RParen)) {
        ++it;
      }
    }
  }
  return {query, constraints};
}

}  // namespace deadfood::parse