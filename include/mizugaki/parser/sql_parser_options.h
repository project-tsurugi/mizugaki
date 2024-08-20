#pragma once

#include <memory>
#include <utility>

#include <mizugaki/parser/sql_parser_element_kind.h>

namespace mizugaki::parser {

class sql_parser_options {
public:
    /// @brief the size type.
    using size_type = std::size_t;

    /// @brief default number of next token candidates to display on error.
    static constexpr std::size_t default_max_expected_candidates = 5;

    /**
     * @brief creates a new instance.
     */
    sql_parser_options();

    /**
     * @brief sets the max number of next token candidates to display on error.
     * @param count the max number of candidates, or 0 to disable to display
     * @see default_max_expected_candidates
     */
    [[nodiscard]] size_type& max_expected_candidates() noexcept;

    /// @copydoc max_expected_candidates()
    [[nodiscard]] size_type const& max_expected_candidates() const noexcept;

    /**
     * @brief returns limits of each syntactic element.
     * @details the limits are used to restrict the number of elements in the AST.
     *      Each default limit is 0, which means no limit.
     * @return the map of limit sof each syntactic element
     */
    [[nodiscard]] sql_parser_element_map<size_type>& element_limits() noexcept;

    /// @copydoc element_limits()
    [[nodiscard]] sql_parser_element_map<size_type> const& element_limits() const noexcept;

    /**
     * @brief returns the debug level.
     * @return the debug level
     * @note this feature is only available for debug configurations
     */
    [[nodiscard]] int& debug() noexcept;

    /// @copydoc debug()
    [[nodiscard]] int const& debug() const noexcept;

private:
    int debug_ {};
    size_type max_expected_candidates_ { default_max_expected_candidates };
    std::unique_ptr<sql_parser_element_map<size_type>> element_limits_;
};

} // namespace mizugaki::parser
