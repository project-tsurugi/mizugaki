#include "shakujo_translator_impl.h"

#include <takatori/scalar/variable_reference.h>

#include <takatori/util/finalizer.h>
#include <takatori/util/downcast.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <shakujo/model/name/SimpleName.h>

#include "details/dml_statement_translator.h"
#include "details/ddl_statement_translator.h"

namespace mizugaki::translator {

using impl = shakujo_translator::impl;
using code_type = shakujo_translator_code;

using ::takatori::util::downcast;
using ::takatori::util::finalizer;
using ::takatori::util::string_builder;

impl::result_type impl::operator()(
        options_type const& options,
        ::shakujo::model::statement::Statement const& statement,
        ::takatori::document::document_map const& documents,
        placeholder_map const& placeholders) {
    initialize(options, documents, placeholders);
    finalizer f { [this] { finalize(); } };
    if (details::dml_statement_translator::is_supported(statement)) {
        details::dml_statement_translator translator { *this };
        auto result = translator.process(statement);
        if (!result) {
            result = std::move(diagnostics_);
        }
        return result;
    }
    if (details::ddl_statement_translator::is_supported(statement)) {
        details::ddl_statement_translator translator { *this };
        auto result = translator.process(statement);
        if (!result) {
            result = std::move(diagnostics_);
        }
        return result;
    }
    diagnostics_.emplace_back(
            code_type::unsupported_statement,
            string_builder {}
                    << "must be a DDL/DML statement: "
                    << statement.kind()
                    << string_builder::to_string,
            region(statement.region()));
    return { std::move(diagnostics_) };
}

impl::options_type const& shakujo_translator::impl::options() const {
    return *options_;
}

std::vector<impl::diagnostic_type>& impl::diagnostics() noexcept {
    return diagnostics_;
}

std::vector<impl::diagnostic_type> const& impl::diagnostics() const noexcept {
    return diagnostics_;
}

static ::takatori::document::position convert0(
        ::shakujo::common::core::DocumentRegion::Position position,
        bool advance) {
    auto line = position.line_number();
    auto column = position.column_number();
    if (line == 0 || column == 0) {
        return {};
    }
    if (advance) {
        return { line, column + 1 };
    }
    return { line, column };
}

::takatori::document::region impl::region(shakujo::common::core::DocumentRegion const& region) const {
    if (documents_ && region) {
        if (auto doc = documents_->find(region.path())) {
            return {
                    *doc,
                    convert0(region.begin(), false),
                    convert0(region.end(), true),
            };
        }
    }
    return {};
}

std::unique_ptr<::takatori::scalar::expression> impl::placeholder(std::string_view name) const {
    if (placeholders_) {
        if (auto ph = placeholders_->find(name)) {
            return ph->resolve();
        }
    }
    if (auto v = options_->host_variable_provider().find(name)) {
        ::yugawara::binding::factory f {};
        return std::make_unique<::takatori::scalar::variable_reference>(f(std::move(v)));
    }
    return {};
}

std::shared_ptr<::yugawara::storage::index const> impl::find_table(std::string_view name) const {
    if (auto table = options().storage_provider().find_table(name)) {
        if (auto index = options().storage_provider().find_primary_index(*table)) {
            return index;
        }
    }
    return {};
}

std::shared_ptr<::yugawara::storage::index const> impl::find_table(::shakujo::model::name::Name const& name) const {
    if (auto const* n = downcast<::shakujo::model::name::SimpleName>(&name); n != nullptr) {
        return find_table(n->token());
    }
    // FIXME: qualified table name
    return {};
}

std::shared_ptr<::yugawara::storage::index const> impl::find_index(std::string_view name) const {
    return options().storage_provider().find_index(name);
}

std::shared_ptr<::yugawara::storage::index const> impl::find_index(::shakujo::model::name::Name const& name) const {
    if (auto const* n = downcast<::shakujo::model::name::SimpleName>(&name); n != nullptr) {
        return find_index(n->token());
    }
    // FIXME: qualified index name
    return {};
}

::yugawara::util::object_repository<::takatori::type::data>& impl::types() noexcept {
    return types_;
}

::yugawara::util::object_repository<::takatori::value::data>& impl::values() noexcept {
    return values_;
}

::yugawara::analyzer::expression_analyzer& shakujo_translator::impl::expression_analyzer() noexcept {
    return expression_analyzer_;
}

::yugawara::analyzer::expression_analyzer const& shakujo_translator::impl::expression_analyzer() const noexcept {
    return expression_analyzer_;
}

std::vector<std::shared_ptr<::takatori::type::data const>>& shakujo_translator::impl::type_buffer() noexcept {
    return type_buffer_;
}

std::vector<std::shared_ptr<::yugawara::aggregate::declaration const>>& shakujo_translator::impl::aggregate_buffer() noexcept {
    return aggregate_buffer_;
}

std::vector<std::shared_ptr<::yugawara::function::declaration const>>& shakujo_translator::impl::function_buffer() noexcept {
    return function_buffer_;
}

impl& impl::extract(shakujo_translator& interface) noexcept {
    return *interface.impl_;
}

impl& impl::initialize(
        options_type const& options,
        ::takatori::document::document_map const& documents,
        placeholder_map const& placeholders) {
    options_ = options;
    documents_ = documents;
    placeholders_ = placeholders;
    types_ = decltype(types_) { true };
    values_ = decltype(values_) { true };
    return *this;
}

void impl::finalize() {
    options_ = nullptr;
    documents_ = nullptr;
    placeholders_ = nullptr;
    types_ = {};
    values_ = {};
    diagnostics_.clear();
    expression_analyzer_.clear_diagnostics();
    expression_analyzer_.variables().clear();
    expression_analyzer_.expressions().clear();
    type_buffer_.clear();
    aggregate_buffer_.clear();
    function_buffer_.clear();
}

} // namespace mizugaki::translator
