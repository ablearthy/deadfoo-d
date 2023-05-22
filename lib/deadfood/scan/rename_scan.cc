#include "rename_scan.hh"

namespace deadfood::scan {

RenameScan::RenameScan(std::unique_ptr<IScan> internal,
                       const std::string& old_name, const std::string& new_name)
    : internal_{std::move(internal)},
      old_name_{old_name},
      new_name_{new_name} {}

void RenameScan::BeforeFirst() { internal_->BeforeFirst(); }

bool RenameScan::Next() { return internal_->Next(); }

bool RenameScan::HasField(const std::string& field_name) const {
  if (field_name == new_name_) {
    return internal_->HasField(old_name_);
  }
  return internal_->HasField(field_name);
}

core::FieldVariant RenameScan::GetField(const std::string& field_name) const {
  if (field_name == new_name_) {
    return internal_->GetField(old_name_);
  }
  return internal_->GetField(field_name);
}

void RenameScan::SetField(const std::string& field_name,
                          const core::FieldVariant& value) {
  if (field_name == new_name_) {
    internal_->SetField(old_name_, value);
  } else {
    internal_->SetField(field_name, value);
  }
}

void RenameScan::Insert() { internal_->Insert(); }
void RenameScan::Delete() { internal_->Delete(); }
void RenameScan::Close() { internal_->Close(); }

}  // namespace deadfood::scan