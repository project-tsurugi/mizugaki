#pragma once

#include <gtest/gtest.h>

#include <takatori/type/data.h>

#include <yugawara/schema/catalog.h>

#include <yugawara/schema/declaration.h>
#include <yugawara/schema/configurable_provider.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/configurable_provider.h>

#include <yugawara/variable/declaration.h>
#include <yugawara/variable/configurable_provider.h>

#include <yugawara/aggregate/declaration.h>
#include <yugawara/aggregate/configurable_provider.h>

#include "../utils.h"

namespace mizugaki::analyzer::details {

class test_parent : public ::testing::Test {
public:
    test_parent();

    analyzer_context& context();

    std::string diagnostics();

    bool find_error(analyzer_context::diagnostic_type::code_type code);

    std::size_t count_error();

    void expect_no_error();

    void clear_error();

    std::shared_ptr<::yugawara::storage::table const> install_table(std::string_view name);

    ::takatori::descriptor::variable vd(
            std::string_view name = "id",
            ::takatori::util::rvalue_ptr<::takatori::type::data> type = {});

protected:
    std::shared_ptr<::yugawara::storage::configurable_provider> storages_;
    std::shared_ptr<::yugawara::aggregate::configurable_provider> set_functions_;
    std::shared_ptr<::yugawara::schema::configurable_provider> schemas_;
    std::shared_ptr<::yugawara::schema::catalog> catalog_;
    std::shared_ptr<::yugawara::schema::declaration> default_schema_;
    std::shared_ptr<::yugawara::schema::search_path> search_path_;

    sql_analyzer_options options_;
    placeholder_map placeholders_ {};
    ::yugawara::variable::configurable_provider host_parameters_ {};

    bool validate_scalar_expressions_ { true };

private:
    analyzer_context context_;
    std::optional<::takatori::util::finalizer> context_finalizer_;
};

} // namespace mizugaki::analyzer::details

