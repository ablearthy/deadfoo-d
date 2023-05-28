#pragma once

#include <deadfood/query/update_query.hh>
#include <deadfood/lex/lex.hh>

namespace deadfood::parse {

query::UpdateQuery ParseUpdateQuery(const std::vector<lex::Token>& tokens);

}