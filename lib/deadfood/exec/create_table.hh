#pragma once

#include <deadfood/database.hh>
#include <deadfood/query/create_table_query.hh>

namespace deadfood::exec {

void ExecuteCreateTableQuery(
    Database& db,
    const std::pair<query::CreateTableQuery, std::vector<core::Constraint>>&
        query);

}