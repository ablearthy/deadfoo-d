#include "insert_into_get_table_scan.hh"

namespace deadfood::expr {

scan::IScan* InsertIntoGetTableScan::GetScan(const std::string& field_name) {
  throw std::runtime_error("cannot access fields while inserting");
}

}  // namespace deadfood::expr