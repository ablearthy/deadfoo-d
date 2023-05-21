#include "product_scan.hh"

namespace deadfood::scan {

ProductScan::ProductScan(std::unique_ptr<IScan> lhs, std::unique_ptr<IScan> rhs)
    : lhs_{std::move(lhs)}, rhs_{std::move(rhs)}, lhs_has_rows{lhs_->Next()} {}

void ProductScan::BeforeFirst() {
  lhs_->BeforeFirst();
  lhs_->Next();
  rhs_->BeforeFirst();
}

bool ProductScan::Next() {
  if (!lhs_has_rows) {
    return false;
  }
  if (rhs_->Next()) {
    return true;
  }
  // no rows in rhs -> shift lhs
  rhs_->BeforeFirst();
  const auto l = lhs_->Next();
  const auto r = rhs_->Next();
  return l && r;
}

bool ProductScan::HasField(const std::string& field_name) const {
  return lhs_->HasField(field_name) || rhs_->HasField(field_name);
}

core::FieldVariant ProductScan::GetField(const std::string& field_name) const {
  if (lhs_->HasField(field_name)) {
    return lhs_->GetField(field_name);
  }
  return rhs_->GetField(field_name);
}

void ProductScan::SetField(const std::string& field_name,
                           const core::FieldVariant& value) {
  if (lhs_->HasField(field_name)) {
    lhs_->SetField(field_name, value);
  } else {
    rhs_->SetField(field_name, value);
  }
}

void ProductScan::Insert() {}
void ProductScan::Delete() {}

void ProductScan::Close() {
  lhs_->Close();
  rhs_->Close();
}

}  // namespace deadfood::scan