#include "expr_tree_parser.hh"
#include <deadfood/parse/parser_error.hh>

namespace deadfood::parse {

template <typename It>
expr::ExprTree ParseExprTreeInternal(It& it, const It end);

template <typename It>
struct my_visitor {
  It& it;
  const It end;
  bool neg_applied, not_applied;

  expr::FactorTree operator()(const lex::Keyword& tok) {
    if (tok == lex::Keyword::Null) {
      ++it;
      return expr::FactorTree{.neg_applied = neg_applied,
                              .not_applied = not_applied,
                              .factor = core::null_t{}};
    }
    throw ParserError("expected `NULL`");
  }
  expr::FactorTree operator()(const lex::Symbol& tok) {
    if (tok == lex::Symbol::LParen) {  // expr
      ++it;
      auto expr = ParseExprTreeInternal(it, end);
      if (it == end || !lex::IsSymbol(*it, lex::Symbol::RParen)) {
        throw ParserError("expected `)`");
      } else {
        ++it;
      }
      return expr::FactorTree{.neg_applied = neg_applied,
                              .not_applied = not_applied,
                              .factor = expr};
    }
    throw ParserError("expected `(`");
  }

  expr::FactorTree operator()(const lex::Identifier& tok) {
    ++it;
    return expr::FactorTree{.neg_applied = neg_applied,
                            .not_applied = not_applied,
                            .factor = tok.id};
  }

  expr::FactorTree operator()(const int& tok) {
    ++it;
    return expr::FactorTree{
        .neg_applied = neg_applied, .not_applied = not_applied, .factor = tok};
  }

  expr::FactorTree operator()(const double& tok) {
    ++it;
    return expr::FactorTree{
        .neg_applied = neg_applied, .not_applied = not_applied, .factor = tok};
  }

  expr::FactorTree operator()(const std::string& tok) {
    expr::Constant constant = tok;
    ++it;
    return expr::FactorTree{.neg_applied = neg_applied,
                            .not_applied = not_applied,
                            .factor = constant};
  }

  expr::FactorTree operator()(const bool& tok) {
    ++it;
    return expr::FactorTree{
        .neg_applied = neg_applied, .not_applied = not_applied, .factor = tok};
  }
};

template <typename It>
expr::FactorTree ParseFactorTree(It& it, const It end) {
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
    throw ParserError("expected input for factor");
  }

  return std::visit(my_visitor<It>{.it = it,
                                   .end = end,
                                   .neg_applied = neg_applied,
                                   .not_applied = not_applied},
                    *it);
}

static const std::vector<expr::GenBinOp> kPrecedence = {
    expr::GenBinOp::LE,  expr::GenBinOp::GE,   expr::GenBinOp::GT,
    expr::GenBinOp::LT,  expr::GenBinOp::Eq,   expr::GenBinOp::And,
    expr::GenBinOp::Or,  expr::GenBinOp::Xor,  expr::GenBinOp::Mul,
    expr::GenBinOp::Div, expr::GenBinOp::Plus, expr::GenBinOp::Minus};

template <typename It>
expr::GenBinOp ParseOp(It& it, const It end) {
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
  } else if (lex::IsSymbol(*it, lex::Symbol::Plus)) {
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
}

template <typename It>
expr::ExprTree ParseExprTreeInternal(It& it, const It end) {
  std::vector<expr::GenBinOp> ops;
  std::vector<expr::FactorTree> factors;
  while (it != end) {
    factors.emplace_back(ParseFactorTree(it, end));
    if (it == end) {
      break;
    }
    ops.emplace_back(ParseOp(it, end));
  }
}

expr::ExprTree ParseExprTree(const std::vector<lex::Token>& tokens) {
  auto it = tokens.cbegin();
  return ParseExprTreeInternal(it, tokens.cend());
}

}  // namespace deadfood::parse
