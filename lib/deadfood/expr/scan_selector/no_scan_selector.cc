#include "no_scan_selector.hh"

namespace deadfood::expr {

scan::IScan* NoScanSelector::GetScan(const std::string& field_name) {
  throw std::runtime_error("cannot access fields while inserting");
}

}  // namespace deadfood::expr