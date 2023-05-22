#pragma once

#include <set>
#include <string>
#include <variant>
#include <vector>
#include <string_view>

namespace deadfood::lex {

static const std::set<std::string> kKeywords = {
    "select", "from",   "where",   "and", "or",     "xor",   "insert",  "into",
    "values", "delete", "update",  "set", "create", "table", "boolean", "int",
    "float",  "double", "varchar", "as",  "join",   "on",    "exists",  "null"};

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
  Null
};

struct Identifier {
  std::string id;
};

using Token = std::variant<int, double, std::string, bool, Identifier, Keyword>;

std::vector<Token> Lex(std::string_view input);

}  // namespace deadfood::lex