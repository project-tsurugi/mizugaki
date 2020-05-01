#include "shakujo_translator_impl.h"

#include <takatori/util/finalizer.h>

#include "details/dml_statement_translator.h"

namespace mizugaki::translator {

using impl = shakujo_translator::impl;

using ::takatori::util::finalizer;

impl::result_type impl::operator()(
        context_type& context,
        ::shakujo::model::statement::Statement const& statement,
        document_map const& documents,
        placeholder_map const& placeholders) {
    initialize(context, documents, placeholders);
    finalizer f { [this] { finalize(); } };
    details::dml_statement_translator translator { *this };
    auto result = translator.process(statement);
    if (!result) {
        result = std::move(diagnostics_);
    }
    return result;
}

impl::context_type& shakujo_translator::impl::context() {
    return *context_;
}

impl::context_type const& shakujo_translator::impl::context() const {
    return *context_;
}

::takatori::util::object_creator shakujo_translator::impl::object_creator() const {
    return context().get_object_creator();
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
        context_type& context,
        document_map const& documents,
        placeholder_map const& placeholders) {
    context_ = context;
    documents_ = documents;
    placeholders_ = placeholders;
    types_ = decltype(types_) { context.get_object_creator() };
    values_ = decltype(values_) { context.get_object_creator() };
    return *this;
}

void impl::finalize() {
    context_ = nullptr;
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
