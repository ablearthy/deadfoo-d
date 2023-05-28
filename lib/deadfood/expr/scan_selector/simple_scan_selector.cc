#include "simple_scan_selector.hh"

namespace deadfood::expr {

SimpleScanSelector::SimpleScanSelector(scan::IScan* internal)
    : internal_{internal} {}

scan::IScan* SimpleScanSelector::GetScan(const std::string& field_name) {
  return internal_;
}

}  // namespace deadfood::expr