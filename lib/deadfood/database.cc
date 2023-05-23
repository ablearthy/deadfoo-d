#include "database.hh"

#include <algorithm>

namespace deadfood {

void RemoveUnnecessaryConstraints(std::vector<core::Constraint>& constraints,
                                  const std::string& slave_table_name) {
  std::remove_if(
      constraints.begin(), constraints.end(),
      [&](const core::Constraint& constr) {
        if (!std::holds_alternative<core::ReferencesConstraint>(constr)) {
          return false;
        }
        const auto& tbl =
            std::get<core::ReferencesConstraint>(constr).slave_table;
        return tbl == slave_table_name;
      });
}

Database::Database(deadfood::storage::DBStorage& storage,
                   std::map<std::string, core::Schema>& schemas,
                   std::vector<core::Constraint>& constraints)
    : storage_{std::move(storage)},
      schemas_{std::move(schemas)},
      constraints_{std::move(constraints)} {}

std::vector<core::Constraint>& Database::constraints() { return constraints_; }

const std::set<std::string>& Database::table_names() const {
  return table_names_;
}

bool Database::Exists(const std::string& table_name) const {
  return schemas_.contains(table_name);
}

void Database::AddTable(const std::string& table_name,
                        const core::Schema& schema) {
  if (Exists(table_name)) {
    return;
  }
  table_names_.emplace(table_name);
  schemas_.emplace(table_name, schema);
  storage_.Add(table_name);
}

void Database::RemoveTable(const std::string& table_name) {
  if (!Exists(table_name)) {
    return;
  }
  table_names_.erase(table_name);
  schemas_.erase(table_name);
  storage_.Remove(table_name);
  RemoveUnnecessaryConstraints(constraints_, table_name);
}

std::unique_ptr<scan::TableScan> Database::GetTableScan(
    const std::string& table_name) {
  auto& schema = schemas_.at(table_name);
  auto& table_storage = storage_.Get(table_name);
  return std::make_unique<scan::TableScan>(table_storage, schema);
}

void Dump(const Database& db, const std::filesystem::path& path) {}

}  // namespace deadfood