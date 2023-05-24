#pragma once

#include <deadfood/expr/expr_tree.hh>
#include <deadfood/lex/lex.hh>

namespace deadfood::parse {

expr::ExprTree ParseExprTree(const std::vector<lex::Token>& tokens);

}  // namespace deadfood::parse
