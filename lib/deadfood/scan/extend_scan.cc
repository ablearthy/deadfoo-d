#include "extend_scan.hh"

namespace deadfood::scan {

ExtendScan::ExtendScan(std::unique_ptr<IScan> internal,
                       std::unique_ptr<expr::IExpr> expr,
                       const std::string& name)
    : internal_{std::move(internal)},
      expr_{std::move(expr)},
      name_{name},
      before_first_{true} {}

void ExtendScan::BeforeFirst() {
  if (internal_ != nullptr) {
    internal_->BeforeFirst();
  }
  before_first_ = true;
}
bool ExtendScan::Next() {
  if (internal_ != nullptr) {
    return internal_->Next();
  }
  if (before_first_) {
    before_first_ = false;
    return true;
  }
  return false;
}

bool ExtendScan::HasField(const std::string& field_name) const {
  if (field_name == name_) {
    return true;
  }
  if (internal_ != nullptr) {
    return internal_->HasField(field_name);
  }
  return false;
}

core::FieldVariant ExtendScan::GetField(const std::string& field_name) const {
  if (field_name == name_) {
    return expr_->Eval();
  }
  return internal_->GetField(field_name);
}

void ExtendScan::SetField(const std::string& field_name,
                          const core::FieldVariant& value) {
  if (field_name != name_) {
    internal_->SetField(field_name, value);
  }
}

void ExtendScan::Insert() { internal_->Insert(); }
void ExtendScan::Delete() { internal_->Delete(); }
void ExtendScan::Close() { internal_->Close(); }

}  // namespace deadfood::scan