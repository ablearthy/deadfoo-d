#pragma once

#include <deadfood/query/delete_query.hh>
#include <deadfood/lex/lex.hh>

namespace deadfood::parse {

query::DeleteQuery
ParseDeleteQuery(const std::vector<lex::Token>& tokens);

}