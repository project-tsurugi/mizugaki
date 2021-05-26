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
#include <shakujo/model/name/Name.h>

#include <mizugaki/translator/shakujo_translator.h>

namespace mizugaki::translator {

class shakujo_translator::impl {
public:
    using options_type = shakujo_translator::options_type;
    using result_type = shakujo_translator::result_type;
    using diagnostic_type = shakujo_translator::diagnostic_type;

    [[nodiscard]] result_type operator()(
            options_type const& options,
            ::shakujo::model::statement::Statement const& statement,
            ::takatori::document::document_map const& documents = {},
            placeholder_map const& placeholders = {});

    [[nodiscard]] options_type const& options() const;

    [[nodiscard]] std::vector<diagnostic_type>& diagnostics() noexcept;
    [[nodiscard]] std::vector<diagnostic_type> const& diagnostics() const noexcept;

    [[nodiscard]] ::takatori::document::region region(::shakujo::common::core::DocumentRegion const& region) const;

    [[nodiscard]] std::unique_ptr<::takatori::scalar::expression> placeholder(std::string_view name) const;

    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::index const> find_table(std::string_view name) const;
    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::index const> find_table(::shakujo::model::name::Name const& name) const;

    [[nodiscard]] ::yugawara::util::object_repository<::takatori::type::data>& types() noexcept;
    [[nodiscard]] ::yugawara::util::object_repository<::takatori::value::data>& values() noexcept;

    [[nodiscard]] ::yugawara::analyzer::expression_analyzer& expression_analyzer() noexcept;
    [[nodiscard]] ::yugawara::analyzer::expression_analyzer const& expression_analyzer() const noexcept;

    [[nodiscard]] std::vector<std::shared_ptr<::takatori::type::data const>>& type_buffer() noexcept;
    [[nodiscard]] std::vector<std::shared_ptr<::yugawara::aggregate::declaration const>>& aggregate_buffer() noexcept;
    [[nodiscard]] std::vector<std::shared_ptr<::yugawara::function::declaration const>>& function_buffer() noexcept;

    impl& initialize(
            options_type const& options,
            ::takatori::document::document_map const& documents = {},
            placeholder_map const& placeholders = {});

    void finalize();

    static impl& extract(shakujo_translator& interface) noexcept;

private:
    ::takatori::util::optional_ptr<options_type const> options_;
    ::takatori::util::optional_ptr<::takatori::document::document_map const> documents_;
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
