#pragma once

#include <deadfood/database.hh>
#include <deadfood/query/select_query.hh>

namespace deadfood::exec {

void ExecuteSelectQuery(Database& db, const query::SelectQuery& query);

}