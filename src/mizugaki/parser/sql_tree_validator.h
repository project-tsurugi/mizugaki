#pragma once

#include <utility>
#include <optional>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/parser/sql_parser_diagnostic.h>

namespace mizugaki::parser {

/**
 * @brief validates SQL AST.
 */
class sql_tree_validator {
public:
    /// @brief the diagnostic type.
    using diagnostic_type = sql_parser_diagnostic;

    /**
     * @brief creates a new instance.
     * @param depth_limit the limit of the depth of the AST, or 0 if it is unlimited
     * @param node_limit the limit of the number of nodes in the AST, or 0 if it is unlimited
     */
    explicit sql_tree_validator(
            std::size_t node_limit,
            std::size_t depth_limit) noexcept;

    /**
     * @brief return whether to the validation is enabled.
     * @return true if the validation is enabled
     * @return false otherwise
     */
    [[nodiscard]] bool enabled() const noexcept;

    /**
     * @brief validates the compilation unit.
     * @param element the target compilation unit
     * @return empty if it is valid
     * @return a diagnostic information if it is invalid
     */
    [[nodiscard]] std::optional<sql_parser_diagnostic> operator()(ast::compilation_unit const& element);

    /**
     * @brief returns the number of nodes in the last validation.
     * @return the number of nodes in the last validation
     * @return 0 if the validation has not been performed
     */
    [[nodiscard]] std::size_t last_node_count() const noexcept;

    /**
     * @brief returns the depth of the AST in the last validation.
     * @return the depth of the AST in the last validation
     * @return 0 if the validation has not been performed
     */
    [[nodiscard]] std::size_t last_max_depth() const noexcept;

private:
    std::size_t node_limit_;
    std::size_t depth_limit_;

    std::size_t last_node_count_ {};
    std::size_t last_max_depth_ {};
};

} // namespace mizugaki::parser
