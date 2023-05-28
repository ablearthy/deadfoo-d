#include "delete.hh"

#include <deadfood/expr/expr_convert.hh>
#include <deadfood/expr/scan_selector/simple_scan_selector.hh>
#include <deadfood/scan/select_scan.hh>
#include <deadfood/expr/bool_expr.hh>

#include <deadfood/exec/dml_util.hh>

namespace deadfood::exec {

void ExecuteDeleteQuery(Database& db, const query::DeleteQuery& query) {
  if (!db.Exists(query.table_name)) {
    throw std::runtime_error("table does not exist");
  }
  auto scan = db.GetTableScan(query.table_name);
  if (query.predicate.has_value()) {
    expr::ExprTreeConverter conv{
        std::make_unique<expr::SimpleScanSelector>(scan.get())};
    scan = std::make_unique<scan::SelectScan>(
        std::move(scan),
        expr::BoolExpr(conv.ConvertExprTreeToIExpr(query.predicate.value())));
  }

  while (scan->Next()) {
    util::CheckForeignKeyConstraintForRow(db, scan.get(), query.table_name,
                                          util::Action::Delete);
    scan->Delete();
  }
}

}  // namespace deadfood::exec