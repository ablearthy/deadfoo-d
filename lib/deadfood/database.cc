#include "database.hh"

#include <algorithm>
#include <istream>
#include <fstream>

#include <deadfood/binary/get.hh>
#include <deadfood/binary/put.hh>

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
      constraints_{std::move(constraints)} {
  for (const auto& [table_name, _] : schemas_) {
    table_names_.emplace(table_name);
  }
}

std::vector<core::Constraint>& Database::constraints() { return constraints_; }

const std::vector<core::Constraint>& Database::constraints_const() const {
  return constraints_;
}

storage::TableStorage& Database::table_storage(const std::string& table_name) {
  return storage_.Get(table_name);
}

const storage::TableStorage& Database::table_storage_const(
    const std::string& table_name) const {
  return storage_.GetConst(table_name);
}

const std::set<std::string>& Database::table_names() const {
  return table_names_;
}

const std::map<std::string, core::Schema>& Database::schemas() const {
  return schemas_;
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
  return std::make_unique<scan::TableScan>(table_storage, schema, table_name);
}

std::unique_ptr<scan::TableScan> Database::GetTableScan(
    const std::string& table_name, const std::string& rename_table) {
  auto& schema = schemas_.at(table_name);
  auto& table_storage = storage_.Get(table_name);
  return std::make_unique<scan::TableScan>(table_storage, schema, rename_table);
}
void DumpSchemas(const std::map<std::string, core::Schema>& schemas,
                 std::ostream& stream) {
  for (const auto& [table_name, schema] : schemas) {
    binary::PutCString(stream, table_name);
    binary::PutUint<uint32_t>(stream, schema.fields().size());
    for (const auto& field : schema.fields()) {
      const auto& field_info = schema.field_info(field);
      binary::PutCString(stream, field);
      binary::PutUint<uint8_t>(stream,
                               static_cast<uint8_t>(schema.MayBeNull(field)));
      binary::PutUint<uint8_t>(stream,
                               static_cast<uint8_t>(schema.IsUnique(field)));
      binary::PutUint<uint8_t>(stream, static_cast<uint8_t>(field_info.type()));
      binary::PutUint<uint32_t>(stream,
                                static_cast<uint32_t>(field_info.size()));
    }
  }
}

void DumpConstraints(const std::vector<core::Constraint>& constraints,
                     std::ostream& stream) {
  for (const auto& constraint : constraints) {
    if (!std::holds_alternative<core::ReferencesConstraint>(constraint)) {
      throw std::runtime_error(
          "a new type of constraint is added, but not handled");
    }
    const auto& ref_constr = std::get<core::ReferencesConstraint>(constraint);
    binary::PutCString(stream, ref_constr.slave_table);
    binary::PutCString(stream, ref_constr.slave_field);
    binary::PutCString(stream, ref_constr.master_table);
    binary::PutCString(stream, ref_constr.master_field);
  }
}

void DumpTable(const storage::TableStorage& storage, std::ostream& stream) {
  for (const auto& [rowid, row_buf] : storage.storage_const()) {
    binary::PutUint<size_t>(stream, rowid);
    binary::PutBytes(stream, row_buf.data(), row_buf.size());
  }
}

void Dump(const Database& db, const std::filesystem::path& path) {
  std::ofstream schema_stream(path / ".schema", std::ios::binary);
  DumpSchemas(db.schemas(), schema_stream);

  std::ofstream constraints_stream(path / ".constraints", std::ios::binary);
  DumpConstraints(db.constraints_const(), constraints_stream);

  for (const auto& table_name : db.table_names()) {
    std::ofstream table_stream(path / (table_name + ".dat"), std::ios::binary);
    const auto row_size = db.schemas().at(table_name).size();
    DumpTable(db.table_storage_const(table_name), table_stream);
  }
}

std::map<std::string, core::Schema> LoadSchemas(std::istream& stream) {
  std::map<std::string, core::Schema> map;
  while (stream.peek(), !stream.eof()) {
    const auto table_name = binary::GetCString(stream);
    const uint32_t fields_count = binary::GetUint<uint32_t>(stream);
    core::Schema schema;
    for (uint32_t i = 0; i < fields_count; ++i) {
      const auto field_name = binary::GetCString(stream);
      const auto may_be_null =
          static_cast<bool>(binary::GetUint<uint8_t>(stream));
      const auto is_unique =
          static_cast<bool>(binary::GetUint<uint8_t>(stream));
      const auto field_type =
          static_cast<core::Field::FieldType>(binary::GetUint<uint8_t>(stream));
      const uint32_t field_size = binary::GetUint<uint32_t>(stream);
      schema.AddField(field_name, core::Field{field_type, field_size},
                      may_be_null, is_unique);
    }
    map.emplace(table_name, std::move(schema));
  }
  return map;
}

std::vector<core::Constraint> LoadConstraint(std::istream& stream) {
  std::vector<core::Constraint> ret;

  while (stream.peek(), !stream.eof()) {
    const auto slave_table = binary::GetCString(stream);
    const auto slave_field = binary::GetCString(stream);
    const auto master_table = binary::GetCString(stream);
    const auto master_field = binary::GetCString(stream);
    ret.emplace_back(core::ReferencesConstraint{
        .master_table = master_table,
        .master_field = master_field,
        .slave_table = slave_table,
        .slave_field = slave_field,
        .on_delete = core::ReferencesConstraint::OnAction::NoAction,
        .on_update = core::ReferencesConstraint::OnAction::NoAction,
    });
  }

  return ret;
}

storage::TableStorage LoadTable(std::istream& stream, const size_t row_size) {
  storage::TableStorage storage;
  auto& internal_storage = storage.storage();
  while (stream.peek(), !stream.eof()) {
    const size_t rowid = binary::GetUint<size_t>(stream);
    auto ptr = binary::GetBytes(stream, row_size);
    internal_storage.emplace(rowid,
                             storage::ByteBuffer{row_size, std::move(ptr)});
  }
  return storage;
}

Database Load(const std::filesystem::path& path) {
  storage::DBStorage db_storage;
  std::ifstream schema_stream(path / ".schema", std::ios::binary);
  auto schemas = LoadSchemas(schema_stream);
  std::ifstream constraints_stream(path / ".constraints", std::ios::binary);
  auto constraints = LoadConstraint(constraints_stream);

  for (const auto& [table_name, _] : schemas) {
    std::ifstream table_stream(path / (table_name + ".dat"), std::ios::binary);
    const auto row_size = schemas.at(table_name).size();
    auto table = LoadTable(table_stream, row_size);
    db_storage.Add(table_name, table);
  }
  Database db{db_storage, schemas, constraints};
  return db;
}

}  // namespace deadfood