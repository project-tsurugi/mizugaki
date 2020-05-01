#pragma once

#include <memory>
#include <optional>

#include <takatori/util/object_creator.h>
#include <takatori/util/optional_ptr.h>

#include <shakujo/model/name/Name.h>

#include <mizugaki/translator/shakujo_translator_context.h>

namespace mizugaki::translator {

class shakujo_translator_context::impl {
public:
    explicit impl(::takatori::util::object_creator creator = {}) noexcept;

    [[nodiscard]] ::takatori::util::object_creator get_object_creator() const;

    [[nodiscard]] ::yugawara::storage::provider const& storages() const noexcept;
    [[nodiscard]] ::yugawara::variable::provider const& variables() const noexcept;
    [[nodiscard]] ::yugawara::function::provider const& functions() const noexcept;
    [[nodiscard]] ::yugawara::aggregate::provider const& aggregates() const noexcept;

    void storages(std::shared_ptr<::yugawara::storage::provider const> provider) noexcept;
    void variables(std::shared_ptr<::yugawara::variable::provider const> provider) noexcept;
    void functions(std::shared_ptr<::yugawara::function::provider const> provider) noexcept;
    void aggregates(std::shared_ptr<::yugawara::aggregate::provider const> provider) noexcept;

    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::index const> find_table(std::string_view name) const;
    [[nodiscard]] ::takatori::util::optional_ptr<::yugawara::storage::index const> find_table(::shakujo::model::name::Name const& name) const;

    static impl& extract(shakujo_translator_context& interface) noexcept;

private:
    ::takatori::util::object_creator creator_;
    std::shared_ptr<::yugawara::storage::provider const> storages_;
    std::shared_ptr<::yugawara::variable::provider const> variables_;
    std::shared_ptr<::yugawara::function::provider const> functions_;
    std::shared_ptr<::yugawara::aggregate::provider const> aggregate_functions_;
};

} // namespace mizugaki::translator
