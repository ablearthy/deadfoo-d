#include "drop_table.hh"

#include <deadfood/core/constraint.hh>

#include <deadfood/scan/iscan.hh>
#include <deadfood/scan/select_scan.hh>


#include <deadfood/expr/cmp_expr.hh>
#include <deadfood/expr/field_expr.hh>
#include <deadfood/expr/exists_expr.hh>
#include <deadfood/expr/bool_expr.hh>
#include <deadfood/expr/iexpr.hh>


namespace deadfood::exec {

bool FindDependentRow(Database& db, const std::string& master_table,
                      const std::string& master_field,
                      const std::string& slave_table,
                      const std::string& slave_field) {
  auto scan_slave = db.GetTableScan(slave_table);
  auto scan_master = db.GetTableScan(master_table);

  // EXISTS(SELECT 1 FROM master_table WHERE EXISTS(SELECT 1 FROM
  // slave_table WHERE slave_table.slave_field = master_table.master_field))

  std::unique_ptr<expr::IExpr> cmp_fields = std::make_unique<expr::CmpExpr>(
      expr::CmpOp::Eq,
      std::make_unique<expr::FieldExpr>(scan_slave.get(), slave_field),
      std::make_unique<expr::FieldExpr>(scan_master.get(), master_field));
  std::unique_ptr<scan::IScan> inner_select =
      std::make_unique<scan::SelectScan>(std::move(scan_slave),
                                         expr::BoolExpr(std::move(cmp_fields)));
  expr::ExistsExpr exists(std::make_unique<scan::SelectScan>(
      std::move(scan_master), expr::BoolExpr(std::make_unique<expr::ExistsExpr>(
                                  std::move(inner_select)))));

  return std::get<bool>(exists.Eval());
}

void ExecuteDropTableQuery(Database& db, const std::string& table_name) {
  if (!db.Exists(table_name)) {
    throw std::runtime_error("table does not exist");
  }
  for (const auto& constr : db.constraints()) {
    if (!std::holds_alternative<core::ReferencesConstraint>(constr)) {
      continue;
    }
    const auto ref_constr = std::get<core::ReferencesConstraint>(constr);
    if (ref_constr.master_table == table_name &&
        ref_constr.on_delete ==
            core::ReferencesConstraint::OnAction::NoAction) {
      if (FindDependentRow(db, ref_constr.master_table, ref_constr.master_field,
                           ref_constr.slave_table, ref_constr.slave_field)) {
        throw std::runtime_error("foreign key violated");
      }
    }
  }
  db.RemoveTable(table_name);
}

}  // namespace deadfood::exec