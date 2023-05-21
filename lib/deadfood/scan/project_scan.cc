#include "project_scan.hh"

namespace deadfood::scan {

ProjectScan::ProjectScan(std::unique_ptr<IScan> internal,
                         const std::unordered_set<std::string>& fields)
    : internal_{std::move(internal)}, fields_{fields} {}

void ProjectScan::BeforeFirst() { internal_->BeforeFirst(); }

bool ProjectScan::Next() { return internal_->Next(); }

bool ProjectScan::HasField(const std::string& field_name) const {
  if (fields_.contains(field_name)) {
    return internal_->HasField(field_name);
  }
  return false;
}

core::FieldVariant ProjectScan::GetField(const std::string& field_name) const {
  if (fields_.contains(field_name)) {
    return internal_->GetField(field_name);
  }
  throw std::runtime_error("no field with name '" + field_name + "'");
}

void ProjectScan::SetField(const std::string& field_name,
                           const core::FieldVariant& value) {
  if (fields_.contains(field_name)) {
    return internal_->SetField(field_name, value);
  }
  throw std::runtime_error("no field with name '" + field_name + "'");
}

void ProjectScan::Insert() { internal_->Insert(); }
void ProjectScan::Delete() { internal_->Delete(); }
void ProjectScan::Close() { internal_->Close(); }

}  // namespace deadfood::scan