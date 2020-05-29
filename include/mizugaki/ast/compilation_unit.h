#pragma once

#include <takatori/document/document.h>

#include <takatori/util/object_creator.h>
#include <takatori/util/maybe_shared_ptr.h>

#include "node_region.h"

namespace mizugaki::ast {

/**
 * @brief represents a compilation unit of the program.
 */
class compilation_unit {
public:
    /// @brief the source document type.
    using document_type = ::takatori::document::document;

    /**
     * @brief returns the source document.
     * @return the source document
     */
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> document() const noexcept;

};

} // namespace mizugaki::ast
