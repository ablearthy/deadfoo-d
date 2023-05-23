#include "db_storage.hh"

namespace deadfood::storage {

TableStorage& DBStorage::Get(const std::string& table_name) {
  return storage_[table_name];
}
bool DBStorage::Exists(const std::string& table_name) const {
  return storage_.contains(table_name);
}

void DBStorage::Add(const std::string& table_name) {
  storage_.emplace(table_name, TableStorage{});
}

}  // namespace deadfood::storage