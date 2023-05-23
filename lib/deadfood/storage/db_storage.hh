#pragma once

#include <unordered_map>
#include <string>

#include <deadfood/storage/table_storage.hh>

namespace deadfood::storage {

class DBStorage {
 public:
  const TableStorage& GetConst(const std::string& table_name) const;
  TableStorage& Get(const std::string& table_name);

  bool Exists(const std::string& table_name) const;

  void Add(const std::string& table_name);
  void Remove(const std::string& table_name);

 private:
  std::unordered_map<std::string, TableStorage> storage_;
};

}  // namespace deadfood::storage
