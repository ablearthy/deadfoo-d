#include "create_table.hh"

#include <deadfood/core/schema.hh>

namespace deadfood::exec {

void ExecuteCreateTableQuery(
    Database& db,
    const std::pair<query::CreateTableQuery, std::vector<core::Constraint>>&
        query) {
  const auto& [q, constraints] = query;
  if (db.Exists(q.table_name())) {
    throw std::runtime_error("table already exists");
  }
  core::Schema schema;
  for (const auto& field_name : q.field_names()) {
    const auto field = q.GetField(field_name);
    schema.AddField(field_name, field, q.MayBeNull(field_name),
                    q.IsUnique(field_name));
  }
  db.AddTable(q.table_name(), schema);
  for (const auto& c : constraints) {
    db.constraints().emplace_back(c);
  }
}

}  // namespace deadfood::exec