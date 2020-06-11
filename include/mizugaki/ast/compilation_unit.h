#pragma once

#include <takatori/document/document.h>

#include <takatori/util/object_creator.h>
#include <takatori/util/maybe_shared_ptr.h>

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

    /**
     * @brief creates a new instance
     * @param statements the top level statements
     * @param document the source document
     */
    explicit compilation_unit(
            common::vector<::takatori::util::unique_object_ptr<statement::statement>> statements,
            ::takatori::util::maybe_shared_ptr<document_type const> document = {}) noexcept;

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
     * @brief returns the source document.
     * @return the source document
     * @return empty if it is not sure
     */
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const>& document() noexcept;

    /// @copydoc document()
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> const& document() const noexcept;

private:
    common::vector<::takatori::util::unique_object_ptr<statement::statement>> statements_;
    ::takatori::util::maybe_shared_ptr<document_type const> document_;
};

} // namespace mizugaki::ast
