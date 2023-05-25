#include "lex.hh"

#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace deadfood::lex {

bool Identifier::operator==(const Identifier& other) const {
  return id == other.id;
}

bool Identifier::operator!=(const Identifier& other) const {
  return id != other.id;
}

bool IsDigit(char c) { return '0' <= c && c <= '9'; }

bool IsAlph(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }

bool IsValidForKeywordOrId(char c) {
  return IsAlph(c) || IsDigit(c) || c == '_' || c == '.';
}

Token ParseNumber(std::string_view& input) {
  int integral_part = 0;

  if (input.empty() || !IsDigit(input[0])) {
    throw std::runtime_error("expected number, got only minus");
  }

  while (!input.empty() && IsDigit(input[0])) {
    integral_part = integral_part * 10 + (input[0] - '0');
    input.remove_prefix(1);
  }

  if (input.empty() || input[0] != '.') {
    return integral_part;
  }

  input.remove_prefix(1);  // consume dot

  double frac_part = 0;
  double mul = 0.1;

  while (!input.empty() && IsDigit(input[0])) {
    frac_part += mul * (input[0] - '0');
    mul *= 0.1;
    input.remove_prefix(1);
  }
  return static_cast<double>(integral_part) + frac_part;
}

std::string ParseString(std::string_view& input) {
  input.remove_prefix(1);  // consume `'`
  std::string ret;

  while (!input.empty() && input[0] != '\'') {
    if (input[0] == '\\') {
      input.remove_prefix(1);
      if (input.empty()) {
        throw std::runtime_error("expected second part of escaping sequence");
      }
      switch (input[0]) {
        case 'n':
          ret += '\n';
          break;
        case 't':
          ret += '\t';
          break;
        case '\\':
          ret += '\\';
          break;
        case '\'':
          ret += '\'';
          break;
        default:
          throw std::runtime_error("unknown escaping sequence `\\" +
                                   std::to_string(input[0]) + "`");
      }
      input.remove_prefix(1);
    } else {
      ret += input[0];
      input.remove_prefix(1);
    }
  }
  if (input.empty()) {
    throw std::runtime_error("expected closing quote");
  } else {
    input.remove_prefix(1);
  }

  return ret;
}

std::vector<Token> Lex(std::string_view input) {
  std::vector<Token> tokens;
  while (!input.empty()) {
    if (input[0] == ' ' || input[0] == '\t' || input[0] == '\n') {
      input.remove_prefix(1);
      continue;
    }
    if (('0' <= input[0] && input[0] <= '9')) {
      auto tok = ParseNumber(input);
      tokens.emplace_back(std::move(tok));
    } else if (input[0] == '\'') {
      auto tok = ParseString(input);
      tokens.emplace_back(std::move(tok));
    } else if (IsAlph(input[0])) {  // keyword or id
      std::string str, lowercase_str;
      while (!input.empty() && IsValidForKeywordOrId(input[0])) {
        str += input[0];
        lowercase_str += static_cast<char>(
            std::tolower(static_cast<unsigned char>(input[0])));
        input.remove_prefix(1);
      }

      if (kKeywords.contains(lowercase_str)) {
        tokens.emplace_back(kKeywordLiteralToKeyword.at(lowercase_str));
      } else {
        tokens.emplace_back(Identifier{str});
      }
    } else {
      switch (input[0]) {
        case '(':
          tokens.emplace_back(Symbol::LParen);
          break;
        case ')':
          tokens.emplace_back(Symbol::RParen);
          break;
        case '=':
          tokens.emplace_back(Symbol::Eq);
          break;
        case '<':
          tokens.emplace_back(Symbol::Less);
          break;
        case '>':
          tokens.emplace_back(Symbol::More);
          break;
        case '+':
          tokens.emplace_back(Symbol::Plus);
          break;
        case '-':
          tokens.emplace_back(Symbol::Minus);
          break;
        case '*':
          tokens.emplace_back(Symbol::Mul);
          break;
        case '/':
          tokens.emplace_back(Symbol::Div);
          break;
        case ',':
          tokens.emplace_back(Symbol::Comma);
          break;
        default:
          std::stringstream ss;
          ss << "unknown symbol `" << input[0] << '`';
          throw std::runtime_error(ss.str());
      }
      input.remove_prefix(1);
    }
  }
  return tokens;
}

bool IsKeyword(const lex::Token& tok, lex::Keyword keyword) {
  return std::holds_alternative<lex::Keyword>(tok) &&
         std::get<lex::Keyword>(tok) == keyword;
}

bool IsSymbol(const lex::Token& tok, lex::Symbol sym) {
  return std::holds_alternative<lex::Symbol>(tok) &&
         std::get<lex::Symbol>(tok) == sym;
}

}  // namespace deadfood::lex