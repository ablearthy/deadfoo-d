#include "dql_util.hh"

#include <deadfood/expr/iexpr.hh>
#include <deadfood/expr/cmp_expr.hh>
#include <deadfood/expr/const_expr.hh>
#include <deadfood/expr/field_expr.hh>

#include <deadfood/scan/select_scan.hh>

namespace deadfood::exec::util {

size_t CountRowsWithMatchingField(Database& db, const std::string& table_name,
                                  const std::string& field_name,
                                  const core::FieldVariant& value,
                                  const size_t limit) {
  auto scan = db.GetTableScan(table_name);

  std::unique_ptr<expr::IExpr> predicate = std::make_unique<expr::CmpExpr>(
      expr::CmpOp::Eq, std::make_unique<expr::ConstExpr>(value),
      std::make_unique<expr::FieldExpr>(scan.get(), field_name));

  scan = std::make_unique<scan::SelectScan>(
      std::move(scan), expr::BoolExpr(std::move(predicate)));

  size_t counter = 0;

  while (scan->Next()) {
    ++counter;
    if (limit != 0 && counter >= limit) {
      break;
    }
  }

  return counter;
}

}  // namespace deadfood::exec::util