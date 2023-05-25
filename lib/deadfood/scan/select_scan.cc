#include "select_scan.hh"

namespace deadfood::scan {

SelectScan::SelectScan(std::unique_ptr<IScan> scan, expr::BoolExpr&& predicate)
    : internal_{std::move(scan)}, predicate_{std::move(predicate)} {}

void SelectScan::set_predicate(expr::BoolExpr&& predicate) {
  predicate_ = std::move(predicate);
}

void SelectScan::BeforeFirst() { internal_->BeforeFirst(); }

bool SelectScan::Next() {
  while (internal_->Next()) {
    const auto v = predicate_.Eval();
    if (std::holds_alternative<core::null_t>(v)) {
      continue;
    }
    if (!std::get<bool>(v)) {
      continue;
    }
    return true;
  }
  return false;
}

bool SelectScan::HasField(const std::string& field_name) const {
  return internal_->HasField(field_name);
}

core::FieldVariant SelectScan::GetField(const std::string& field_name) const {
  return internal_->GetField(field_name);
}

void SelectScan::SetField(const std::string& field_name,
                          const core::FieldVariant& value) {
  return internal_->SetField(field_name, value);
}

void SelectScan::Insert() { return internal_->Insert(); }

void SelectScan::Delete() { return internal_->Delete(); }

void SelectScan::Close() { return internal_->Close(); }

}  // namespace deadfood::scan