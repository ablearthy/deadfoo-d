#pragma once

#include <vector>

#include <deadfood/lex/lex.hh>
#include <deadfood/query/insert_query.hh>

namespace deadfood::parse {

query::InsertQuery ParseInsertQuery(const std::vector<lex::Token>& tokens);

}