#pragma once

#include <takatori/document/document.h>

#include <takatori/util/maybe_shared_ptr.h>
#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/statement/statement.h>

namespace mizugaki::ast {

/**
 * @brief represents a compilation unit of the program.
 */
class compilation_unit {
public:
    /// @brief the source document type.
    using document_type = ::takatori::document::document;

    /// @brief the region type of element.
    using region_type = node_region;

    /**
     * @brief creates a new instance
     * @param statements the top level statements
     * @param comments the comment regions
     * @param document the source document
     */
    explicit compilation_unit(
            common::vector<::takatori::util::unique_object_ptr<statement::statement>> statements,
            common::vector<region_type> comments = {},
            ::takatori::util::maybe_shared_ptr<document_type const> document = {}) noexcept;

    /**
     * @brief creates a new instance
     * @param statements the top level statements
     * @param comments the comment regions
     * @attention this will take copies of elements
     */
    compilation_unit(
            common::rvalue_list<statement::statement> statements,
            std::initializer_list<region_type> comments = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit compilation_unit(compilation_unit const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit compilation_unit(compilation_unit&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the top level statements.
     * @return the top level statements
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<statement::statement>>& statements() noexcept;

    /// @copydoc statements()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<statement::statement>> const& statements() const noexcept;

    /**
     * @brief returns the comment regions.
     * @return the comment regions
     */
    [[nodiscard]] common::vector<region_type>& comments() noexcept;

    /// @copydoc comments()
    [[nodiscard]] common::vector<region_type> const& comments() const noexcept;

    /**
     * @brief returns the source document.
     * @return the source document
     * @return empty if it is not sure
     */
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const>& document() noexcept;

    /// @copydoc document()
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> const& document() const noexcept;

    /**
     * @brief compares two values.
     * @details This only treats holding statements.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(compilation_unit const& a, compilation_unit const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(compilation_unit const& a, compilation_unit const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            compilation_unit const& value);

private:
    common::vector<::takatori::util::unique_object_ptr<statement::statement>> statements_;
    common::vector<region_type> comments_;
    ::takatori::util::maybe_shared_ptr<document_type const> document_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, compilation_unit const& value);

} // namespace mizugaki::ast
