#pragma once

#include <deadfood/query/select_query.hh>

namespace deadfood::parse {

template <std::forward_iterator It>
query::SelectQuery ParseSelectQuery(It& it, const It end);

}