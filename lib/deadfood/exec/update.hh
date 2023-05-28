#pragma once

#include <deadfood/database.hh>
#include <deadfood/query/update_query.hh>

namespace deadfood::exec {

void ExecuteUpdateQuery(Database& db, const query::UpdateQuery& query);

}  // namespace deadfood::exec
