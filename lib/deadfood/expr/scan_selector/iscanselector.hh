#pragma once

#include <string>
#include <deadfood/scan/iscan.hh>

namespace deadfood::expr {

class IScanSelector {
 public:
  virtual scan::IScan* GetScan(const std::string& field_name) = 0;

  virtual ~IScanSelector() = default;
};

}  // namespace deadfood::expr