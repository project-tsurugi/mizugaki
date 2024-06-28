#pragma once

#include <memory>

#include <takatori/document/document.h>

#include <takatori/util/optional_ptr.h>
#include <takatori/util/sequence_view.h>

#include <yugawara/variable/provider.h>

#include <mizugaki/placeholder_map.h>
#include <mizugaki/ast/compilation_unit.h>
#include <mizugaki/ast/statement/statement.h>

#include "sql_analyzer_options.h"
#include "sql_analyzer_result.h"

namespace mizugaki::analyzer {

/**
 * @brief analyzes AST and convert it into the corresponded takatori IR.
 */
class sql_analyzer {
public:
    /// @private
    class impl;

    /// @brief the options type.
    using options_type = sql_analyzer_options;

    /// @brief the result type.
    using result_type = sql_analyzer_result;

    /**
     * @brief creates a new instance.
     */
    sql_analyzer();

    ~sql_analyzer();

    sql_analyzer(sql_analyzer const& other) = delete;
    sql_analyzer& operator=(sql_analyzer const& other) = delete;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    sql_analyzer(sql_analyzer&& other) noexcept;

    /**
     * @brief assigns the given object into this.
     * @param other the copy source
     */
    sql_analyzer& operator=(sql_analyzer&& other) noexcept;

    /**
     * @brief parses a statement.
     * @param options the analysis options
     * @param statement the source statement
     * @param source the source program
     * @param placeholders the input placeholders
     * @param host_parameters the host parameter declarations
     * @return the analysis result
     * @return invalid result if an error was occurred
     */
    [[nodiscard]] result_type operator()(
            options_type const& options,
            ast::statement::statement const& statement,
            ast::compilation_unit const& source,
            placeholder_map const& placeholders = {},
            ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters = {});

    /**
     * @brief parses a statement.
     * @param options the analysis options
     * @param statement the source statement
     * @param source the source document
     * @param comments the comment regions in the document
     * @param placeholders the input placeholders
     * @param host_parameters the host variables for placeholders: each entry name may start with `:`
     * @return the analysis result
     * @return invalid result if an error was occurred
     */
    [[nodiscard]] result_type operator()(
            options_type const& options,
            ast::statement::statement const& statement,
            ::takatori::util::optional_ptr<::takatori::document::document const> source = {},
            ::takatori::util::sequence_view<ast::node_region const> comments = {},
            placeholder_map const& placeholders = {},
            ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters = {});

private:
    std::unique_ptr<impl> impl_;
};

} // namespace mizugaki::analyzer
