#include <stdexcept>
#include "lex.hh"

namespace deadfood::lex {

bool IsDigit(char c) { return '0' <= c && c <= '9'; }

template <typename T>
T WithSign(T num, bool is_neg) {
  if (is_neg) {
    return -num;
  }
  return num;
}

Token ParseNumber(std::string_view& input) {
  bool is_neg = false;
  int integral_part = 0;

  if (input[0] == '-' || input[0] == '+') {
    is_neg = (input[0] == '-');
    input.remove_prefix(1);
  }

  if (input.empty() || !IsDigit(input[0])) {
    throw std::runtime_error("expected number, got only minus");
  }

  while (!input.empty() && IsDigit(input[0])) {
    integral_part = integral_part * 10 + (input[0] - '0');
    input.remove_prefix(1);
  }

  if (input.empty() || input[0] != '.') {
    return WithSign(integral_part, is_neg);
  }

  input.remove_prefix(1);  // consume dot

  double frac_part = 0;
  double mul = 0.1;

  while (!input.empty() && IsDigit(input[0])) {
    frac_part += mul * (input[0] - '0');
    mul *= 0.1;
    input.remove_prefix(1);
  }
  return WithSign(static_cast<double>(integral_part) + frac_part, is_neg);
}

std::string ParseString(std::string_view input) {
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

  return ret;
}

std::vector<Token> Lex(std::string_view input) {
  std::vector<Token> tokens;
  while (!input.empty()) {
    if (input[0] == ' ' || input[0] == '\t' || input[0] == '\n') {
      input.remove_prefix(1);
      continue;
    }
    if (input[0] == '-' || input[0] == '+' ||
        ('0' <= input[0] && input[0] <= '9')) {
      auto tok = ParseNumber(input);
      tokens.emplace_back(std::move(tok));
    } else if (input[0] == '\'') {
      auto tok = ParseString(input);
      tokens.emplace_back(std::move(tok));
    } else {
      // id or keyword
    }
  }
  return tokens;
}

}  // namespace deadfood::lex