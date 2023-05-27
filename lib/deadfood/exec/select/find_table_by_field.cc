#include "find_table_by_field.hh"

#include <deadfood/util/parse.hh>

namespace deadfood::exec {

void ThrowErrorIfHasCollision(
    const std::optional<FindTableByFieldResult>& candidate,
    const std::string& field_name) {
  if (candidate.has_value()) {
    throw std::runtime_error("ambiguous source for `" + field_name + "` field");
  }
}

std::string GetFullFieldName(const std::string& table_name,
                             const std::optional<std::string>& alias,
                             const std::string& field_name) {
  if (alias.has_value()) {
    return alias.value() + "." + field_name;
  }
  return table_name + "." + field_name;
}

std::optional<FindTableByFieldResult> FindTableByFieldInternal(
    const std::map<std::string, core::Schema>& schemas,
    const std::string& field_name, const std::string& table_name,
    const std::optional<std::string>& alias) {
  query::FromTable from_table{.table_name = table_name, .renamed = alias};
  const auto [tbl, field] = deadfood::parse::util::GetFullFieldName(field_name);
  const auto& schema = schemas.at(table_name);

  bool field_must_exist = false;
  if (tbl.has_value()) {
    const auto tbl_equals_table_name = tbl.value() == table_name;
    const auto tbl_equals_alias =
        (alias.has_value() && tbl.value() == alias.value());
    if (tbl_equals_alias || tbl_equals_table_name) {
      field_must_exist = true;
    }
  }
  if (field_must_exist && !schema.Exists(field)) {
    throw std::runtime_error("cannot find field `" + field + "` in table `" +
                             table_name + "`");
  }
  if (!schema.Exists(field)) {
    return std::nullopt;
  }

  return FindTableByFieldResult{
      .full_field_name = GetFullFieldName(table_name, alias, field),
      .from_table = std::make_optional(from_table)};
}

struct SelectSourceVisitor {
  const std::map<std::string, core::Schema>& schemas;
  const std::string& field_name;

  std::optional<FindTableByFieldResult> operator()(
      const query::FromTable& from_table) {
    return FindTableByFieldInternal(schemas, field_name, from_table.table_name,
                                    from_table.renamed);
  }
  std::optional<FindTableByFieldResult> operator()(
      const query::SelectQuery& select_query) {
    return FindTableByField(schemas, select_query, field_name);
  }
};

std::optional<FindTableByFieldResult> FindTableByField(
    const std::map<std::string, core::Schema>& schemas,
    const query::SelectQuery& query, const std::string& field_name) {
  std::optional<FindTableByFieldResult> candidate{std::nullopt};
  for (const auto& source : query.sources) {
    SelectSourceVisitor vis{schemas, field_name};
    if (auto another_candidate = std::visit(vis, source)) {
      ThrowErrorIfHasCollision(candidate, field_name);
      candidate = another_candidate;
    }
  }

  for (const auto& join : query.joins) {
    if (auto another_candidate = FindTableByFieldInternal(
            schemas, field_name, join.table_name, join.alias)) {
      ThrowErrorIfHasCollision(candidate, field_name);
      candidate = another_candidate;
    }
  }

  return candidate;
}

}  // namespace deadfood::exec