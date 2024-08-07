#pragma once

#include <vector>

#include <tsl/hopscotch_map.h>

#include <takatori/util/optional_ptr.h>
#include <takatori/util/sequence_view.h>

#include <yugawara/storage/relation.h>

#include <mizugaki/analyzer/details/relation_info.h>

#include "find_element_result.h"

namespace mizugaki::analyzer::details {

class query_scope {
public:
    using result_type = find_element_result<relation_info>;
    using const_result_type = find_element_result<relation_info const>;
    using position_type = std::size_t;

    query_scope() = default;
    explicit query_scope(::takatori::util::optional_ptr<query_scope const> parent);

    [[nodiscard]] ::takatori::util::optional_ptr<query_scope const>& parent() noexcept;
    [[nodiscard]] ::takatori::util::optional_ptr<query_scope const> parent() const noexcept;
    [[nodiscard]] ::takatori::util::sequence_view<relation_info> references() noexcept;
    [[nodiscard]] ::takatori::util::sequence_view<relation_info const> references() const noexcept;

    void reserve(std::size_t size);
    relation_info& add(relation_info info);

    [[nodiscard]] result_type find(std::string_view identifier);
    [[nodiscard]] const_result_type find(std::string_view identifier) const;
    [[nodiscard]] result_type find(::yugawara::storage::relation const& relation);
    [[nodiscard]] const_result_type find(::yugawara::storage::relation const& relation) const;

    [[nodiscard]] position_type create_pivot() const noexcept;

private:
    static constexpr position_type ambiguous = std::numeric_limits<position_type>::max();

    ::takatori::util::optional_ptr<query_scope const> parent_ {};
    std::vector<relation_info> relations_ {};
    ::tsl::hopscotch_map<std::string, position_type, std::hash<std::string_view>, std::equal_to<>> name_map_ {};
    ::tsl::hopscotch_map<yugawara::storage::relation const*, position_type> reference_map_ {};

    // FIXME: support WITH ...

    [[nodiscard]] std::optional<position_type> find_internal(std::string_view identifier) const;
    [[nodiscard]] std::optional<position_type> find_internal(::yugawara::storage::relation const& relation) const;
};

std::ostream& operator<<(std::ostream& out, query_scope const& value);

} // namespace mizugaki::analyzer::details
