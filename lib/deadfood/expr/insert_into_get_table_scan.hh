#pragma once

#include <deadfood/expr/expr_convert.hh>

namespace deadfood::expr {

class InsertIntoGetTableScan : public GetTableScan {
 public:
  scan::IScan* GetScan(const std::string& field_name) override;
};

}  // namespace deadfood
