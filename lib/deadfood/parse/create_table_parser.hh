#pragma once

#include <deadfood/core/constraint.hh>
#include <deadfood/query/create_table_query.hh>
#include <deadfood/lex/lex.hh>

#include <stdexcept>

namespace deadfood::parse {

std::pair<query::CreateTableQuery, std::vector<core::Constraint>>
ParseCreateTableQuery(const std::vector<lex::Token>& tokens);

}  // namespace deadfood::parse