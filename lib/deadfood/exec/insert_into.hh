#pragma once

#include <deadfood/database.hh>
#include <deadfood/query/insert_query.hh>

namespace deadfood::exec {

void ExecuteInsertQuery(Database& db, const query::InsertQuery& query);

}