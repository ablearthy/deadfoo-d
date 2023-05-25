#pragma once

#include <deadfood/expr/expr_tree.hh>
#include <deadfood/lex/lex.hh>
#include <deadfood/parse/parser_error.hh>

#include <optional>
#include <vector>
#include <algorithm>

namespace deadfood::parse {

template <typename It>
expr::FactorTree ParseExprTreeInternal(It& it, const It end);

template <typename It>
struct my_visitor {
  It& it;
  const It end;
  bool neg_applied, not_applied;

  std::optional<expr::FactorTree> operator()(const lex::Keyword& tok) {
    if (tok == lex::Keyword::Null) {
      ++it;
      return expr::FactorTree{.neg_applied = neg_applied,
                              .not_applied = not_applied,
                              .factor = core::null_t{}};
    }
    return std::nullopt;
  }
  std::optional<expr::FactorTree> operator()(const lex::Symbol& tok) {
    if (tok == lex::Symbol::LParen) {  // expr
      ++it;
      auto expr = ParseExprTreeInternal(it, end);
      if (it == end || !lex::IsSymbol(*it, lex::Symbol::RParen)) {
        throw ParserError("expected `)`");
      } else {
        ++it;
      }
      return expr;
    }
    return std::nullopt;
  }

  std::optional<expr::FactorTree> operator()(const lex::Identifier& tok) {
    ++it;
    return expr::FactorTree{.neg_applied = neg_applied,
                            .not_applied = not_applied,
                            .factor = tok.id};
  }

  std::optional<expr::FactorTree> operator()(const int& tok) {
    ++it;
    return expr::FactorTree{
        .neg_applied = neg_applied, .not_applied = not_applied, .factor = tok};
  }

  std::optional<expr::FactorTree> operator()(const double& tok) {
    ++it;
    return expr::FactorTree{
        .neg_applied = neg_applied, .not_applied = not_applied, .factor = tok};
  }

  std::optional<expr::FactorTree> operator()(const std::string& tok) {
    expr::Constant constant = tok;
    ++it;
    return expr::FactorTree{.neg_applied = neg_applied,
                            .not_applied = not_applied,
                            .factor = constant};
  }

  std::optional<expr::FactorTree> operator()(const bool& tok) {
    ++it;
    return expr::FactorTree{
        .neg_applied = neg_applied, .not_applied = not_applied, .factor = tok};
  }
};

template <typename It>
std::optional<expr::FactorTree> ParseFactorTree(It& it, const It end) {
  bool not_applied = false;
  bool neg_applied = false;
  if (it != end && lex::IsKeyword(*it, lex::Keyword::Not)) {
    not_applied = true;
    ++it;
  }
  if (it != end && lex::IsSymbol(*it, lex::Symbol::Minus)) {
    neg_applied = true;
    ++it;
  }
  if (it == end) {
    if ((not_applied || neg_applied)) {
      throw ParserError("expected input for factor");
    }
    return std::nullopt;
  }

  return std::visit(my_visitor<It>{.it = it,
                                   .end = end,
                                   .neg_applied = neg_applied,
                                   .not_applied = not_applied},
                    *it);
}

static const std::vector<std::vector<expr::GenBinOp>> kPrecedence = {
    {expr::GenBinOp::And},
    {expr::GenBinOp::Or},
    {expr::GenBinOp::Xor},
    {expr::GenBinOp::LE, expr::GenBinOp::GE, expr::GenBinOp::GT,
     expr::GenBinOp::LT, expr::GenBinOp::Eq, expr::GenBinOp::Is,
     expr::GenBinOp::IsNot},
    {expr::GenBinOp::Plus, expr::GenBinOp::Minus},
    {expr::GenBinOp::Mul, expr::GenBinOp::Div}};

template <typename It>
std::optional<expr::GenBinOp> ParseOp(It& it, const It end) {
  if (lex::IsSymbol(*it, lex::Symbol::Less)) {
    ++it;
    if (it == end) {
      throw ParserError("unexpected end");
    }
    if (lex::IsSymbol(*it, lex::Symbol::Eq)) {
      ++it;
      return expr::GenBinOp::LE;

    } else {
      return expr::GenBinOp::LT;
    }
  } else if (lex::IsSymbol(*it, lex::Symbol::More)) {
    ++it;
    if (it == end) {
      throw ParserError("unexpected end");
    }
    if (lex::IsSymbol(*it, lex::Symbol::Eq)) {
      ++it;
      return expr::GenBinOp::GE;
    } else {
      return expr::GenBinOp::GT;
    }
  } else if (lex::IsKeyword(*it, lex::Keyword::Is)) {
    ++it;
    if (it == end) {
      throw ParserError("unexpected end");
    }
    if (lex::IsKeyword(*it, lex::Keyword::Not)) {
      ++it;
      return expr::GenBinOp::IsNot;
    } else {
      return expr::GenBinOp::Is;
    }
  } else if (lex::IsSymbol(*it, lex::Symbol::Eq)) {
    ++it;
    return expr::GenBinOp::Eq;
  } else if (lex::IsSymbol(*it, lex::Symbol::Mul)) {
    ++it;
    return expr::GenBinOp::Mul;
  } else if (lex::IsSymbol(*it, lex::Symbol::Div)) {
    ++it;
    return expr::GenBinOp::Div;
  } else if (lex::IsSymbol(*it, lex::Symbol::Plus)) {
    ++it;
    return expr::GenBinOp::Plus;
  } else if (lex::IsSymbol(*it, lex::Symbol::Minus)) {
    ++it;
    return expr::GenBinOp::Minus;
  } else if (lex::IsKeyword(*it, lex::Keyword::And)) {
    ++it;
    return expr::GenBinOp::And;
  } else if (lex::IsKeyword(*it, lex::Keyword::Or)) {
    ++it;
    return expr::GenBinOp::Or;
  } else if (lex::IsKeyword(*it, lex::Keyword::Xor)) {
    ++it;
    return expr::GenBinOp::Xor;
  }
  return std::nullopt;
}

template <typename It, typename ItOp>
expr::FactorTree BuildTree(It begin_factors, It end_factors, ItOp begin_op,
                           ItOp end_op, size_t op_idx) {
  if (op_idx == kPrecedence.size()) {
    if (begin_factors == end_factors) {
      throw ParserError("expected some term");
    }
    return *begin_factors;
  }
  std::vector<expr::FactorTree> factors;
  expr::FactorTree ret;

  expr::GenBinOp cur_op;

  auto last_op = begin_op;
  auto last_factor = begin_factors;
  auto it_op = begin_op;
  size_t factors_count = 1;
  for (; it_op != end_op; ++it_op) {
    const auto& prec = kPrecedence[op_idx];
    const auto maybe_it = std::find(prec.cbegin(), prec.cend(), *it_op);
    if (maybe_it == prec.cend()) {
      ++factors_count;
    } else {
      cur_op = *it_op;
      break;
    }
  }
  auto factor = BuildTree(last_factor, last_factor + factors_count, last_op,
                          it_op, op_idx + 1);
  factors.emplace_back(factor);
  last_factor += factors_count;
  if (it_op == end_op) {
    last_op = it_op;
  } else {
    last_op = it_op + 1;
  }
  factors_count = 1;

  while (last_factor != end_factors) {
    if (it_op != end_op) {
      ++it_op;
    }
    for (; it_op != end_op; ++it_op) {
      const auto& prec = kPrecedence[op_idx];
      const auto maybe_it = std::find(prec.begin(), prec.end(), *it_op);
      if (maybe_it == prec.end()) {
        ++factors_count;
      } else {
        break;
      }
    }

    factor = BuildTree(last_factor, last_factor + factors_count, last_op, it_op,
                       op_idx + 1);
    factors.emplace_back(factor);
    last_factor += factors_count;
    if (it_op == end_op) {
      last_op = it_op;
    } else {
      last_op = it_op + 1;
    }
    factors_count = 1;

    if (cur_op != *it_op) {
      auto expr = expr::ExprTree{.op = cur_op, .factors = factors};
      factors.clear();
      factors.emplace_back(expr::FactorTree{
          .neg_applied = false, .not_applied = false, .factor = expr});
      cur_op = *it_op;
    }
  }

  if (factors.size() == 1) {
    return factors[0];
  }
  return expr::FactorTree{
      .neg_applied = false,
      .not_applied = false,
      .factor = expr::ExprTree{.op = cur_op, .factors = factors}};
}

template <typename It>
expr::FactorTree ParseExprTreeInternal(It& it, const It end) {
  std::vector<expr::GenBinOp> ops;
  std::vector<expr::FactorTree> factors;
  while (it != end) {
    if (auto factor = ParseFactorTree(it, end)) {
      factors.emplace_back(factor.value());
    } else {
      break;
    }

    if (it == end) {
      break;
    }
    if (auto op = ParseOp(it, end)) {
      ops.emplace_back(op.value());
    } else {
      break;
    }
  }
  if (ops.size() + 1 != factors.size()) {
    throw ParserError("invalid expression");
  }

  return BuildTree(factors.begin(), factors.end(), ops.begin(), ops.end(), 0);
}

template <std::forward_iterator It>
  requires std::is_same_v<typename It::value_type, lex::Token>
expr::FactorTree ParseExprTree(It& it, const It end) {
  return ParseExprTreeInternal(it, end);
}

}  // namespace deadfood::parse
