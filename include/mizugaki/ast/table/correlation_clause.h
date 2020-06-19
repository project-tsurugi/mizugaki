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
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit correlation_clause() = default;

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

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(correlation_clause const& a, correlation_clause const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(correlation_clause const& a, correlation_clause const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            correlation_clause const& value);

private:
    common::clone_wrapper<::takatori::util::unique_object_ptr<name::simple>> correlation_name_ {};
    common::clone_wrapper<common::vector<::takatori::util::unique_object_ptr<name::simple>>> column_names_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, correlation_clause const& value);

} // namespace mizugaki::ast::table
