#pragma once

#include <atomic>

#include <takatori/type/data.h>
#include <takatori/value/data.h>

#include <takatori/scalar/expression.h>

#include <takatori/relation/expression.h>

#include <takatori/util/finalizer.h>

#include <yugawara/compiled_info.h>

#include <yugawara/analyzer/expression_analyzer.h>

#include <yugawara/util/object_repository.h>

#include <mizugaki/analyzer/sql_analyzer.h>

namespace mizugaki::analyzer::details {

class analyzer_context {
public:
    using options_type = sql_analyzer::options_type;
    using result_type = sql_analyzer::result_type;
    using diagnostic_type = result_type::diagnostic_type;

    [[nodiscard]] ::takatori::util::finalizer initialize(
            options_type const& options,
            ::takatori::util::optional_ptr<::takatori::document::document const> source = {},
            ::takatori::util::sequence_view<ast::node_region const> comments = {},
            placeholder_map const& placeholders = {},
            ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters = {});

    [[nodiscard]] ::takatori::util::optional_ptr<options_type const> options() const noexcept {
        return options_;
    }

    [[nodiscard]] ::takatori::util::optional_ptr<::takatori::document::document const> source() const noexcept {
        return source_;
    }

    [[nodiscard]] ::takatori::util::sequence_view<ast::node_region const> comments() const noexcept {
        return comments_;
    }

    [[nodiscard]] ::takatori::util::optional_ptr<placeholder_map const> placeholders() const noexcept {
        return placeholders_;
    }

    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters() const noexcept {
        return host_parameters_;
    }

    [[nodiscard]] std::vector<result_type::diagnostic_type>& diagnostics() noexcept {
        return diagnostics_;
    }

    [[nodiscard]] std::vector<result_type::diagnostic_type> const& diagnostics() const noexcept {
        return diagnostics_;
    }

    [[nodiscard]] ::yugawara::util::object_repository<::takatori::type::data>& types() noexcept {
        return types_;
    }

    [[nodiscard]] ::yugawara::util::object_repository<::takatori::value::data>& values() noexcept {
        return values_;
    }

    [[nodiscard]] std::shared_ptr<::takatori::type::data const> resolve(
            ::takatori::scalar::expression const& expression,
            bool validate = false);

    [[nodiscard]] bool resolve(::takatori::relation::expression const& expression, bool validate = false);

    void resolve_as(
            ::takatori::descriptor::variable const& variable,
            ::yugawara::analyzer::variable_resolution resolution);

    [[nodiscard]] ::yugawara::compiled_info test_info();

    void clear_expression_resolution();

    void clear_expression_resolution(::takatori::scalar::expression const& expression);

    [[nodiscard]] ::takatori::document::region convert(ast::node_region region) const;

    template<class T>
    [[nodiscard]] T bless(T t, ast::node_region region) const {
        t.region() = convert(region);
        return t;
    }

    void report(
            diagnostic_type::code_type code,
            diagnostic_type::message_type message,
            ::takatori::document::region region);

    void report(
            diagnostic_type::code_type code,
            diagnostic_type::message_type message,
            ast::node_region region);

    template<class T, class... Args>
    [[nodiscard]] std::unique_ptr<T> create(::takatori::document::region region, Args&&... args) const {
        auto r = std::make_unique<T>(std::forward<Args>(args)...);
        r->region() = region;
        return r;
    }

    template<class T, class... Args>
    [[nodiscard]] std::unique_ptr<T> create(ast::node_region region, Args&&... args) const {
        return create<T>(convert(region), std::forward<Args>(args)...);
    }

private:
    std::atomic<bool> initialized_ { false };

    ::takatori::util::optional_ptr<options_type const> options_ {};
    ::takatori::util::optional_ptr<::takatori::document::document const> source_ {};
    ::takatori::util::sequence_view<ast::node_region const> comments_ {};
    ::takatori::util::optional_ptr<placeholder_map const> placeholders_ {};
    ::takatori::util::optional_ptr<::yugawara::variable::provider const> host_parameters_ {};

    std::vector<diagnostic_type> diagnostics_ {};
    ::yugawara::util::object_repository<::takatori::type::data> types_ {};
    ::yugawara::util::object_repository<::takatori::value::data> values_ {};
    ::yugawara::analyzer::expression_analyzer expression_analyzer_ {};

    void finalize();

    static sql_analyzer_code convert_code(
            ::yugawara::analyzer::expression_analyzer_code code) noexcept;

};

} // namespace mizugaki::analyzer::details
