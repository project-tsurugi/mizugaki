#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

test_parent::test_parent() :
        storages_ {
                std::make_shared<::yugawara::storage::configurable_provider>(),
        },
        set_functions_ {
                std::make_shared<::yugawara::aggregate::configurable_provider>(),
        },
        schemas_ {
                std::make_shared<::yugawara::schema::configurable_provider>(),
        },
        catalog_ {
                std::make_shared<::yugawara::schema::catalog>(
                        "tsurugi",
                        std::nullopt,
                        schemas_)
        },
        default_schema_ {
                std::make_shared<::yugawara::schema::declaration>(
                        "public",
                        std::nullopt,
                        storages_,
                        std::shared_ptr<::yugawara::variable::provider> {},
                        std::shared_ptr<::yugawara::function::provider> {},
                        set_functions_)
        },
        search_path_ {
                std::make_shared<::yugawara::schema::search_path>(
                        ::yugawara::schema::search_path::vector_type { default_schema_ })
        },
        options_ {
                catalog_,
                search_path_,
                default_schema_,
        }
{
    schemas_->add(default_schema_);
}

analyzer_context &test_parent::context() {
    if (!context_finalizer_) {
        options_.validate_scalar_expressions() = validate_scalar_expressions_;
        context_finalizer_ = context_.initialize(options_, {}, {}, placeholders_, host_parameters_);
    }
    return context_;
}

std::string test_parent::diagnostics() {
    return diagnostics_to_string(context_);
}

bool test_parent::find_error(analyzer_context::diagnostic_type::code_type code) {
    return contains(context_, code);
}

std::shared_ptr<::yugawara::storage::table const> test_parent::install_table(std::string_view name) {
    auto table = storages_->add_table(::yugawara::storage::table {
            name,
            {
                    {
                            "k",
                            ttype::int8 {},
                    },
                    {
                            "v",
                            ttype::character { ttype::varying },
                            ::yugawara::variable::nullable,
                            tvalue::character { "V" },
                    },
                    {
                            "w",
                            ttype::character { ttype::varying },
                            ::yugawara::variable::nullable,
                            tvalue::character { "W" },
                    },
                    {
                            "x",
                            ttype::character { ttype::varying },
                            ::yugawara::variable::nullable,
                            tvalue::character { "X" },
                    },
            },
    });
    storages_->add_index(::yugawara::storage::index {
            table,
            name,
            {
                    {
                            table->columns()[0],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::primary,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    return table;
}

::takatori::descriptor::variable
test_parent::vd(std::string_view name, ::takatori::util::rvalue_ptr<ttype::data> type) {
    auto result = ::yugawara::binding::factory {}.stream_variable(name);
    if (type) {
        context_.resolve_as(result, type.value());
    }
    return result;
}

} // namespace mizugaki::analyzer::details
