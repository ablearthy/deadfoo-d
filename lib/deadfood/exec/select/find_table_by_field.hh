#pragma once

#include <string>
#include <optional>

#include <deadfood/query/select_query.hh>
#include <deadfood/core/schema.hh>

namespace deadfood::exec {

struct FindTableByFieldResult {
  std::string full_field_name;
  std::optional<query::FromTable> from_table;
};

std::optional<FindTableByFieldResult> FindTableByField(
    const std::map<std::string, core::Schema>& schemas,
    const query::SelectQuery& query, const std::string& field_name);

}  // namespace deadfood::exec