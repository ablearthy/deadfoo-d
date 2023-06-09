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
  Database() = default;

  Database(storage::DBStorage& storage,
           std::map<std::string, core::Schema>& schemas,
           std::vector<core::Constraint>& constraints);

  std::vector<core::Constraint>& constraints();
  const std::vector<core::Constraint>& constraints_const() const;

  storage::TableStorage& table_storage(const std::string& table_name);
  const storage::TableStorage& table_storage_const(
      const std::string& table_name) const;
  const std::set<std::string>& table_names() const;
  const std::map<std::string, core::Schema>& schemas() const;
  [[nodiscard]] bool Exists(const std::string& table_name) const;

  void AddTable(const std::string& table_name, const core::Schema& schema);
  void RemoveTable(const std::string& table_name);

  std::unique_ptr<scan::IScan> GetTableScan(const std::string& table_name);
  std::unique_ptr<scan::IScan> GetTableScan(
      const std::string& table_name, const std::string& rename_table);

 private:
  storage::DBStorage storage_;
  std::set<std::string> table_names_;
  std::map<std::string, core::Schema> schemas_;
  std::vector<core::Constraint> constraints_;
};

void Dump(const Database& db, const std::filesystem::path& path);

Database Load(const std::filesystem::path& path);

}  // namespace deadfood
