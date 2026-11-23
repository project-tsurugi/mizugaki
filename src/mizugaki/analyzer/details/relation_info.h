#pragma once

#include <string>
#include <vector>
#include <limits>

#include <tsl/hopscotch_map.h>

#include <takatori/util/optional_ptr.h>
#include <takatori/util/sequence_view.h>

#include <yugawara/storage/relation.h>
#include <yugawara/storage/index.h>

#include <mizugaki/analyzer/details/column_info.h>

#include "find_element_result.h"

namespace mizugaki::analyzer::details {

/**
 * @brief the condition of columns to find in relation_info::find().
 */
enum class column_filter {
    /// @brief all columns.
    all,
    /// @brief only exported columns, which can be referred by unqualified names.
    exported,
};

class relation_info {
public:
    using position_type = std::size_t;
    static constexpr position_type ambiguous = std::numeric_limits<position_type>::max();

    relation_info() = default;

    relation_info( // NOLINT(*-explicit-constructor, *-explicit-conversions)
            ::takatori::util::optional_ptr<::yugawara::storage::relation const> declaration,
            std::string identifier = {});

    ~relation_info() = default;
    relation_info(relation_info const&) = delete;
    relation_info(relation_info&&) noexcept = default;
    relation_info& operator=(relation_info const&) = delete;
    relation_info& operator=(relation_info&&) noexcept = default;


    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::relation const>& declaration() noexcept;
    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::relation const> declaration() const noexcept;
    [[nodiscard]] std::shared_ptr<::yugawara::storage::index const> primary_index() const;
    [[nodiscard]] std::string& identifier() noexcept;
    [[nodiscard]] std::string const& identifier() const noexcept;
    [[nodiscard]] ::takatori::util::sequence_view<column_info> columns() noexcept;
    [[nodiscard]] ::takatori::util::sequence_view<column_info const> columns() const noexcept;

    void reserve(std::size_t count, bool also_declaration_map = false);
    column_info& add(column_info info);
    void erase(position_type first, position_type last);

    /**
     * @brief finds a column with the given name, only from the columns which satisfy the filter.
     * @details this filters the columns and decides whether the result is ambiguous at once,
     *      by using the current flags of the columns (they may be changed after the columns were added).
     *      Callers should not decide the ambiguity by themselves, or it may contradict the filter.
     * @param identifier the column name
     * @param filter the condition of columns
     * @return the found column
     * @return ambiguous if there are two or more such columns
     * @return absent if there are no such columns
     */
    [[nodiscard]] find_element_result<column_info> find(std::string_view identifier, column_filter filter);

    /// @copydoc find(std::string_view, column_filter)
    [[nodiscard]] find_element_result<column_info const> find(std::string_view identifier, column_filter filter) const;

    [[nodiscard]] find_element_result<column_info> find(::yugawara::storage::column const& column);
    [[nodiscard]] find_element_result<column_info const> find(::yugawara::storage::column const& column) const;

    void rebuild();

private:
    ::takatori::util::optional_ptr<::yugawara::storage::relation const> declaration_ {};
    std::string identifier_ {};
    std::vector<column_info> columns_ {};
    ::tsl::hopscotch_map<std::string, position_type, std::hash<std::string_view>, std::equal_to<>> name_map_ {};
    ::tsl::hopscotch_map<::yugawara::storage::column const*, position_type> declaration_map_ {};

    column_info& build_internal(position_type position);
    [[nodiscard]] std::optional<position_type> find_internal(std::string_view identifier, column_filter filter) const;
    [[nodiscard]] std::optional<position_type> find_internal(::yugawara::storage::column const& column) const;
};

std::ostream& operator<<(std::ostream& out, relation_info const& value);

} // namespace mizugaki::analyzer::details
