#pragma once

#include <type_traits>

namespace deadfood::util {

template <typename T>
using IsNumberT =
    std::disjunction<std::is_same<T, bool>, std::is_same<T, int>,
                     std::is_same<T, float>, std::is_same<T, double>>;

}