#pragma once

#include <deadfood/database.hh>
#include <deadfood/query/delete_query.hh>

namespace deadfood::exec {

void ExecuteDeleteQuery(Database& db, const query::DeleteQuery& query);

}