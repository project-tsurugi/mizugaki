#pragma once

#include <memory>

#include <takatori/document/document.h>
#include <takatori/document/document_map.h>

#include <takatori/util/optional_ptr.h>

#include <yugawara/diagnostic.h>

#include <shakujo/model/statement/Statement.h>

#include <mizugaki/placeholder_map.h>

#include "shakujo_translator_options.h"
#include "shakujo_translator_code.h"
#include "shakujo_translator_result.h"

namespace mizugaki::translator {

/**
 * @brief translates DML statement of shakujo IR into the corresponded takatori IR.
 */
class shakujo_translator {
public:
    /// @private
    class impl;

    /// @brief the options type.
    using options_type = shakujo_translator_options;

    /// @brief the result type.
    using result_type = shakujo_translator_result;

    /// @brief the diagnostic information type.
    using diagnostic_type = result_type::diagnostic_type;

    /**
     * @brief creates a new instance.
     */
    shakujo_translator();

    ~shakujo_translator();

    shakujo_translator(shakujo_translator const& other) = delete;
    shakujo_translator& operator=(shakujo_translator const& other) = delete;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    shakujo_translator(shakujo_translator&& other) noexcept;

    /**
     * @brief assigns the given object into this.
     * @param other the copy source
     */
    shakujo_translator& operator=(shakujo_translator&& other) noexcept;

    /**
     * @brief translates the shakujo's statement into corresponded takatori's representation.
     * @details the input statement must be a kind of DML statement, that is, it must be one of the following:
     *
     *      - EmitStatement
     *      - InsertRelationStatement
     *      - InsertValuesStatement
     *      - UpdateStatement
     *      - DeleteStatement
     *
     *      If error occurred during translation, this function will return an invalid result.
     *
     * @param options the translation options
     * @param statement the translation target
     * @param documents the source document map
     * @param placeholders the placeholder map
     * @return the translation result
     * @return invalid result if an error was occurred
     */
    [[nodiscard]] result_type operator()(
            options_type const& options,
            ::shakujo::model::statement::Statement const& statement,
            ::takatori::document::document_map const& documents = {},
            placeholder_map const& placeholders = {});

private:
    std::unique_ptr<impl> impl_;
    friend impl;
};

} // namespace mizugaki::translator
