#pragma once

#include <deadfood/expr/expr_convert.hh>

namespace deadfood::expr {

class NoScanSelector : public IScanSelector {
 public:
  scan::IScan* GetScan(const std::string& field_name) override;
};

}  // namespace deadfood
