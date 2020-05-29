#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>

namespace mizugaki::ast::table {

/**
 * @brief exposes table reference name.
 * @note `7.6 <table reference>`
 */
class correlation_clause : public element {
public:
    /**
     * @brief creates a new instance.
     * @param correlation_name the correlation name
     * @param column_names the correlation column names
     * @param region the element region
     */
    explicit correlation_clause(
            ::takatori::util::unique_object_ptr<name::simple> correlation_name,
            common::vector<::takatori::util::unique_object_ptr<name::simple>> column_names = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit correlation_clause(correlation_clause const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit correlation_clause(correlation_clause&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the correlation name.
     * @return the correlation name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& correlation_name() noexcept;

    /// @copydoc correlation_name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& correlation_name() const noexcept;

    /**
     * @brief returns the correlation column names.
     * @return the column names
     * @return empty if they are not defined explicitly
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>>& column_names() noexcept;

    /// @copydoc column_names()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>> const& column_names() const noexcept;

private:
    common::clone_wrapper<::takatori::util::unique_object_ptr<name::simple>> correlation_name_;
    common::clone_wrapper<common::vector<::takatori::util::unique_object_ptr<name::simple>>> column_names_;
};

} // namespace mizugaki::ast::table
