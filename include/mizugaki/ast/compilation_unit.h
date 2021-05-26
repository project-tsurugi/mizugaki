#pragma once

#include <takatori/document/document.h>

#include <takatori/util/clone_tag.h>
#include <takatori/util/maybe_shared_ptr.h>

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
            std::vector<std::unique_ptr<statement::statement>> statements,
            std::vector<region_type> comments = {},
            ::takatori::util::maybe_shared_ptr<document_type const> document = {}) noexcept;

    /**
     * @brief creates a new instance
     * @param statements the top level statements
     * @param comments the comment regions
     * @attention this will take copies of elements
     */
    compilation_unit(
            common::rvalue_list<statement::statement> statements,
            std::initializer_list<region_type> comments = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit compilation_unit(::takatori::util::clone_tag_t, compilation_unit const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit compilation_unit(::takatori::util::clone_tag_t, compilation_unit&& other);

    /**
     * @brief returns the top level statements.
     * @return the top level statements
     */
    [[nodiscard]] std::vector<std::unique_ptr<statement::statement>>& statements() noexcept;

    /// @copydoc statements()
    [[nodiscard]] std::vector<std::unique_ptr<statement::statement>> const& statements() const noexcept;

    /**
     * @brief returns the comment regions.
     * @return the comment regions
     */
    [[nodiscard]] std::vector<region_type>& comments() noexcept;

    /// @copydoc comments()
    [[nodiscard]] std::vector<region_type> const& comments() const noexcept;

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
    std::vector<std::unique_ptr<statement::statement>> statements_;
    std::vector<region_type> comments_;
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
