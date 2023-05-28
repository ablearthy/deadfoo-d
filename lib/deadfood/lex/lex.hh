#pragma once

#include <set>
#include <string>
#include <variant>
#include <vector>
#include <string_view>
#include <map>

namespace deadfood::lex {

static const std::set<std::string> kKeywords = {
    "select",  "from",   "where",  "and",     "or",      "xor",    "insert",
    "into",    "values", "delete", "update",  "set",     "create", "table",
    "boolean", "int",    "float",  "double",  "varchar", "as",     "join",
    "on",      "exists", "null",   "primary", "foreign", "key",    "references",
    "unique",  "not",    "drop"};

enum class Keyword {
  Select,
  From,
  Where,
  And,
  Or,
  Xor,
  Insert,
  Into,
  Values,
  Delete,
  Update,
  Set,
  Create,
  Table,
  Boolean,
  Int,
  Float,
  Double,
  Varchar,
  As,
  Join,
  On,
  Exists,
  Null,
  Primary,
  Foreign,
  Key,
  References,
  Unique,
  Not,
  Drop,
  Is
};

static std::map<std::string, Keyword> kKeywordLiteralToKeyword = {
    {"select", Keyword::Select},   {"from", Keyword::From},
    {"where", Keyword::Where},     {"and", Keyword::And},
    {"or", Keyword::Or},           {"xor", Keyword::Xor},
    {"insert", Keyword::Insert},   {"into", Keyword::Into},
    {"values", Keyword::Values},   {"delete", Keyword::Delete},
    {"update", Keyword::Update},   {"set", Keyword::Set},
    {"create", Keyword::Create},   {"table", Keyword::Table},
    {"boolean", Keyword::Boolean}, {"int", Keyword::Int},
    {"float", Keyword::Float},     {"double", Keyword::Double},
    {"varchar", Keyword::Varchar}, {"as", Keyword::As},
    {"join", Keyword::Join},       {"on", Keyword::On},
    {"exists", Keyword::Exists},   {"null", Keyword::Null},
    {"primary", Keyword::Primary}, {"foreign", Keyword::Foreign},
    {"key", Keyword::Key},         {"references", Keyword::References},
    {"unique", Keyword::Unique},   {"not", Keyword::Not},
    {"drop", Keyword::Drop},       {"is", Keyword::Is}};

enum class Symbol {
  LParen,
  RParen,
  Eq,
  Less,
  More,
  Plus,
  Minus,
  Mul,
  Div,
  Comma
};

static std::map<char, Symbol> kCharToSymbol = {
    {'(', Symbol::LParen}, {')', Symbol::RParen}, {'=', Symbol::Eq},
    {'<', Symbol::Less},   {'>', Symbol::More},   {'+', Symbol::Plus},
    {'-', Symbol::Minus},  {'*', Symbol::Mul},    {'/', Symbol::Div},
    {',', Symbol::Comma}};

struct Identifier {
  std::string id;

  bool operator==(const Identifier& other) const;
  bool operator!=(const Identifier& other) const;
};

using Token =
    std::variant<int, double, std::string, bool, Identifier, Keyword, Symbol>;

std::vector<Token> Lex(std::string_view input);

bool IsKeyword(const lex::Token& tok, lex::Keyword keyword);

bool IsSymbol(const lex::Token& tok, lex::Symbol sym);

}  // namespace deadfood::lex