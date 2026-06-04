#pragma once

#include <tuple>
#include <vector>

#include <tsl/hopscotch_map.h>

#include <takatori/descriptor/variable.h>

#include <takatori/util/optional_ptr.h>
#include <takatori/util/sequence_view.h>

#include <yugawara/storage/relation.h>

#include <mizugaki/analyzer/details/relation_info.h>

#include "find_element_result.h"
#include "query_info.h"

namespace mizugaki::analyzer::details {

class query_scope {
public:
    using result_type = find_element_result<relation_info>;
    using const_result_type = find_element_result<relation_info const>;
    using position_type = std::size_t;

    query_scope() = default;
    explicit query_scope(::takatori::util::optional_ptr<query_scope> parent);

    [[nodiscard]] ::takatori::util::optional_ptr<query_scope>& parent() noexcept;
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

    /**
     * @brief adds a query info with the given name.
     * @param name the query name
     * @param query the query info
     * @return true if successfully added
     * @return false if there is already a query with the same name in this scope
     */
    [[nodiscard]] bool add(std::string name, std::shared_ptr<query_info const> query);

    /**
     * @brief finds a query info with the given name in this scope.
     * @attention this does not look up from the parent scopes.
     * @param name the query name
     * @return the found query info
     * @return empty if not found
     */
    [[nodiscard]] std::shared_ptr<query_info const> find_query(std::string_view name) const;

    /**
     * @brief whether this scope is independent query scope.
     * @details independent query scope cannot refer variables declared in the parent scopes.
     * @return true if this scope is independent
     * @return false otherwise
     */
    [[nodiscard]] bool& independent_scope() noexcept {
        return independent_scope_;
    }

    /// @copydoc independent_scope()
    [[nodiscard]] bool independent_scope() const noexcept {
        return independent_scope_;
    }

    /**
     * @brief whether to capture parameters in this query scope.
     * @return true if this scope captures variables in parent scope as parameters
     * @return false otherwise
     */
    [[nodiscard]] bool& capture_parameters() noexcept;

    /// @copydoc capture_parameters()
    [[nodiscard]] bool capture_parameters() const noexcept;

    /**
     * @brief list pairs of free variable, and its parameter variable in this scope.
     * @return a vector of tuples, each containing a free variable and its corresponding parameter variable
     */
    [[nodiscard]] std::vector<std::tuple<::takatori::descriptor::variable, ::takatori::descriptor::variable>>&
    list_parameters();

    /// @copydoc list_parameters()
    [[nodiscard]] std::vector<std::tuple<::takatori::descriptor::variable, ::takatori::descriptor::variable>> const&
    list_parameters() const;

    /**
     * @brief adds a query parameter variable for the given free variable.
     * @details if a parameter variable is already added for this scope, this returns the existing parameter variable
     *      with not declaring a new one.
     * @param free_variable the free variable appeared in the parent scope
     * @return the parameter variable for the free variable
     */
    [[nodiscard]] ::takatori::descriptor::variable& add_parameter(
        ::takatori::descriptor::variable const& free_variable);

private:
    static constexpr position_type ambiguous = std::numeric_limits<position_type>::max();

    ::takatori::util::optional_ptr<query_scope> parent_ {};
    std::vector<relation_info> relations_ {};
    ::tsl::hopscotch_map<std::string, position_type, std::hash<std::string_view>, std::equal_to<>> name_map_ {};
    ::tsl::hopscotch_map<yugawara::storage::relation const*, position_type> reference_map_ {};
    ::tsl::hopscotch_map<
            std::string,
            std::shared_ptr<query_info const>,
            std::hash<std::string_view>,
            std::equal_to<>> query_map_ {};

    bool independent_scope_ { false };
    bool capture_parameters_ { false };
    std::vector<std::tuple<::takatori::descriptor::variable, ::takatori::descriptor::variable>> parameters_ {};

    [[nodiscard]] std::optional<position_type> find_internal(std::string_view identifier) const;
    [[nodiscard]] std::optional<position_type> find_internal(::yugawara::storage::relation const& relation) const;
};

std::ostream& operator<<(std::ostream& out, query_scope const& value);

} // namespace mizugaki::analyzer::details
