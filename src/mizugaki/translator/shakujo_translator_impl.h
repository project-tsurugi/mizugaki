#pragma once

#include <memory>
#include <vector>

#include <takatori/type/data.h>
#include <takatori/value/data.h>
#include <takatori/scalar/expression.h>
#include <takatori/document/region.h>

#include <yugawara/analyzer/expression_analyzer.h>
#include <yugawara/aggregate/declaration.h>
#include <yugawara/function/declaration.h>
#include <yugawara/util/object_repository.h>

#include <shakujo/common/core/DocumentRegion.h>

#include <mizugaki/translator/shakujo_translator.h>

#include "shakujo_translator_context_impl.h"

namespace mizugaki::translator {

class shakujo_translator::impl {
public:
    using context_type = shakujo_translator::context_type::impl;
    using result_type = shakujo_translator::result_type;
    using diagnostic_type = shakujo_translator::diagnostic_type;

    [[nodiscard]] result_type operator()(
            context_type& context,
            ::shakujo::model::statement::Statement const& statement,
            document_map const& documents = {},
            placeholder_map const& placeholders = {});

    [[nodiscard]] context_type& context();
    [[nodiscard]] context_type const& context() const;
    [[nodiscard]] ::takatori::util::object_creator object_creator() const;

    [[nodiscard]] std::vector<diagnostic_type>& diagnostics() noexcept;
    [[nodiscard]] std::vector<diagnostic_type> const& diagnostics() const noexcept;

    [[nodiscard]] ::takatori::document::region region(::shakujo::common::core::DocumentRegion const& region) const;

    [[nodiscard]] ::takatori::util::unique_object_ptr<::takatori::scalar::expression> placeholder(std::string_view name) const;

    [[nodiscard]] ::yugawara::util::object_repository<::takatori::type::data>& types() noexcept;
    [[nodiscard]] ::yugawara::util::object_repository<::takatori::value::data>& values() noexcept;

    [[nodiscard]] ::yugawara::analyzer::expression_analyzer& expression_analyzer() noexcept;
    [[nodiscard]] ::yugawara::analyzer::expression_analyzer const& expression_analyzer() const noexcept;

    [[nodiscard]] std::vector<std::shared_ptr<::takatori::type::data const>>& type_buffer() noexcept;
    [[nodiscard]] std::vector<std::shared_ptr<::yugawara::aggregate::declaration const>>& aggregate_buffer() noexcept;
    [[nodiscard]] std::vector<std::shared_ptr<::yugawara::function::declaration const>>& function_buffer() noexcept;

    impl& initialize(
            context_type& context,
            document_map const& documents = {},
            placeholder_map const& placeholders = {});

    void finalize();

    static impl& extract(shakujo_translator& interface) noexcept;

private:
    ::takatori::util::optional_ptr<context_type> context_;
    ::takatori::util::optional_ptr<document_map const> documents_;
    ::takatori::util::optional_ptr<placeholder_map const> placeholders_;
    std::vector<diagnostic_type> diagnostics_;

    ::yugawara::util::object_repository<::takatori::type::data> types_;
    ::yugawara::util::object_repository<::takatori::value::data> values_;

    ::yugawara::analyzer::expression_analyzer expression_analyzer_;
    std::vector<std::shared_ptr<::takatori::type::data const>> type_buffer_;
    std::vector<std::shared_ptr<::yugawara::aggregate::declaration const>> aggregate_buffer_;
    std::vector<std::shared_ptr<::yugawara::function::declaration const>> function_buffer_;
};

} // namespace mizugaki::translator
