#include "field_expr.hh"

namespace deadfood::expr {

FieldExpr::FieldExpr(scan::IScan* scan, const std::string& field_name)
    : scan_{scan}, field_name_{field_name} {}

core::FieldVariant FieldExpr::Eval() { return scan_->GetField(field_name_); }

}  // namespace deadfood::expr