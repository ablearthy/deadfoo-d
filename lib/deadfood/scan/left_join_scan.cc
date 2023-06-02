#include "left_join_scan.hh"

#include <deadfood/expr/cmp_expr.hh>
#include <deadfood/expr/field_expr.hh>

namespace deadfood::scan {

LeftJoinScan::LeftJoinScan(std::unique_ptr<IScan> lhs,
                           std::unique_ptr<IScan> rhs,
                           std::unique_ptr<expr::IExpr> expr)
    : lhs_{std::move(lhs)},
      rhs_{std::move(rhs)},
      predicate_{expr::BoolExpr(std::move(expr))},
      lhs_has_row_{lhs_->Next()},
      cur_lhs_has_rhs_{false},
      rhs_null_{false} {}

void LeftJoinScan::BeforeFirst() {
  lhs_->BeforeFirst();
  lhs_has_row_ = lhs_->Next();
  rhs_->BeforeFirst();
  cur_lhs_has_rhs_ = false;
}

bool FindMatchingRhs(IScan* rhs, expr::BoolExpr& predicate) {
  while (rhs->Next()) {
    auto result = predicate.Eval();
    if (std::holds_alternative<core::null_t>(result) ||
        !std::get<bool>(result)) {
      continue;
    }
    return true;
  }
  return false;
}

bool LeftJoinScan::Next() {
  if (!lhs_has_row_) {
    return false;
  }

  if (rhs_null_) {
    rhs_null_ = false;
    if (!lhs_->Next()) {
      return false;
    }
    cur_lhs_has_rhs_ = false;
    rhs_->BeforeFirst();
  }

  if (FindMatchingRhs(rhs_.get(), predicate_)) {
    cur_lhs_has_rhs_ = true;
    return true;
  } else if (cur_lhs_has_rhs_) {
    if (!lhs_->Next()) {
      return false;
    }
    cur_lhs_has_rhs_ = false;
    rhs_->BeforeFirst();
    if (FindMatchingRhs(rhs_.get(), predicate_)) {
      cur_lhs_has_rhs_ = true;
      return true;
    }
  }

  rhs_null_ = true;
  return true;
}

bool LeftJoinScan::HasField(const std::string& field_name) const {
  return lhs_->HasField(field_name) || rhs_->HasField(field_name);
}

core::FieldVariant LeftJoinScan::GetField(const std::string& field_name) const {
  if (lhs_->HasField(field_name)) {
    return lhs_->GetField(field_name);
  }

  if (rhs_->HasField(field_name) && !rhs_null_) {
    return rhs_->GetField(field_name);
  }
  return core::null_t{};
}

void LeftJoinScan::SetField(const std::string& field_name,
                            const core::FieldVariant& value) {
  if (lhs_->HasField(field_name)) {
    lhs_->SetField(field_name, value);
    return;
  }
  rhs_->SetField(field_name, value);
}

void LeftJoinScan::Insert() {}
void LeftJoinScan::Delete() {}

void LeftJoinScan::Close() {
  lhs_->Close();
  rhs_->Close();
}

void LeftJoinScan::set_predicate(std::unique_ptr<expr::IExpr> expr) {
  predicate_ = expr::BoolExpr(std::move(expr));
}

}  // namespace deadfood::scan