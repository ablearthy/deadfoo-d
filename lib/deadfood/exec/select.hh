#pragma once

#include <deadfood/database.hh>
#include <deadfood/query/select_query.hh>

namespace deadfood::exec {

std::pair<std::unique_ptr<scan::IScan>, std::vector<std::string>>
ExecuteSelectQuery(Database& db, const query::SelectQuery& query);

}