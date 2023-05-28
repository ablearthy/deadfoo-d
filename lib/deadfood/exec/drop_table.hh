#pragma once

#include <deadfood/database.hh>
#include <string>

namespace deadfood::exec {

void ExecuteDropTableQuery(Database& db, const std::string& table_name);

}  // namespace deadfood
