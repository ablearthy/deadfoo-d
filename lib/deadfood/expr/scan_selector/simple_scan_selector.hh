#pragma once

#include <deadfood/expr/scan_selector/iscanselector.hh>

namespace deadfood::expr {

class SimpleScanSelector : public IScanSelector {
 public:
  explicit SimpleScanSelector(scan::IScan* internal);

  scan::IScan* GetScan(const std::string& field_name) override;

 private:
  scan::IScan* internal_;
};

}  // namespace deadfood::expr
