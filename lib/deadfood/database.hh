#pragma once

#include <map>
#include <filesystem>

#include <deadfood/storage/db_storage.hh>
#include <deadfood/core/schema.hh>
#include <deadfood/core/constraint.hh>
#include <deadfood/scan/table_scan.hh>
#include <set>

namespace deadfood {

class Database {
 public:
  Database(storage::DBStorage& storage,
           std::map<std::string, core::Schema>& schemas,
           std::vector<core::Constraint>& constraints);

  std::vector<core::Constraint>& constraints();

  const std::set<std::string>& table_names() const;
  [[nodiscard]] bool Exists(const std::string& table_name) const;

  void AddTable(const std::string& table_name, const core::Schema& schema);
  void RemoveTable(const std::string& table_name);

  std::unique_ptr<scan::TableScan> GetTableScan(const std::string& table_name);

 private:
  storage::DBStorage storage_;
  std::set<std::string> table_names_;
  std::map<std::string, core::Schema> schemas_;
  std::vector<core::Constraint> constraints_;
};

void Dump(const Database& db, const std::filesystem::path& path);

Database Load(const std::filesystem::path& path);

}  // namespace deadfood
