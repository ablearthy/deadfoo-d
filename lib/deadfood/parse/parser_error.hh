#pragma once

#include <stdexcept>

namespace deadfood::parse {

class ParserError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

}  // namespace deadfood::parse