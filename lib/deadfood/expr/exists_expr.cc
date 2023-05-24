#include "exists_expr.hh"

namespace deadfood::expr {

ExistsExpr::ExistsExpr(std::unique_ptr<scan::IScan> scan)
    : internal_{std::move(scan)} {}

core::FieldVariant ExistsExpr::Eval() {
  internal_->BeforeFirst();
  const bool exists = internal_->Next();
  return exists;
}

}  // namespace deadfood::expr