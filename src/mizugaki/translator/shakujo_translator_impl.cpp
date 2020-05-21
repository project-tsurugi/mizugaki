#include "shakujo_translator_impl.h"

#include <takatori/util/finalizer.h>
#include <takatori/util/downcast.h>

#include <shakujo/model/name/SimpleName.h>

#include "details/dml_statement_translator.h"

namespace mizugaki::translator {

using impl = shakujo_translator::impl;

using ::takatori::util::downcast;
using ::takatori::util::finalizer;

impl::result_type impl::operator()(
        options_type const& options,
        ::shakujo::model::statement::Statement const& statement,
        document_map const& documents,
        placeholder_map const& placeholders) {
    initialize(options, documents, placeholders);
    finalizer f { [this] { finalize(); } };
    details::dml_statement_translator translator { *this };
    auto result = translator.process(statement);
    if (!result) {
        result = std::move(diagnostics_);
    }
    return result;
}

impl::options_type const& shakujo_translator::impl::options() const {
    return *options_;
}

::takatori::util::object_creator shakujo_translator::impl::object_creator() const {
    return options().get_object_creator();
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

::takatori::util::unique_object_ptr<::takatori::scalar::expression> impl::placeholder(std::string_view name) const {
    if (placeholders_) {
        if (auto ph = placeholders_->find(name)) {
            return ph->resolve(object_creator());
        }
    }
    return {};
}

::takatori::util::optional_ptr<::yugawara::storage::index const> impl::find_table(std::string_view name) const {
    if (auto table = options().storages().find_table(name)) {
        if (auto index = options().storages().find_primary_index(*table)) {
            return *index;
        }
    }
    return {};
}

::takatori::util::optional_ptr<::yugawara::storage::index const> impl::find_table(::shakujo::model::name::Name const& name) const {
    if (auto const* n = downcast<::shakujo::model::name::SimpleName>(&name); n != nullptr) {
        return find_table(n->token());
    }
    // FIXME: qualified table name
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
        document_map const& documents,
        placeholder_map const& placeholders) {
    options_ = options;
    documents_ = documents;
    placeholders_ = placeholders;
    types_ = decltype(types_) { options.get_object_creator(), true };
    values_ = decltype(values_) { options.get_object_creator(), true };
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
