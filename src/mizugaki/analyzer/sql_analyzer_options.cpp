#include <mizugaki/analyzer/sql_analyzer_options.h>

namespace mizugaki::analyzer {

using ::takatori::util::maybe_shared_ptr;

sql_analyzer_options::sql_analyzer_options(
        maybe_shared_ptr<::yugawara::schema::catalog const> catalog,
        maybe_shared_ptr<::yugawara::schema::search_path const> schema_search_path,
        maybe_shared_ptr<::yugawara::schema::declaration const> default_schema) noexcept :
    catalog_ { std::move(catalog) },
    schema_search_path_ { std::move(schema_search_path) },
    default_schema_ { std::move(default_schema) }
{}

::yugawara::schema::catalog const& sql_analyzer_options::catalog() const noexcept {
    if (catalog_) {
        return *catalog_;
    }
    using namespace std::string_view_literals;
    static ::yugawara::schema::catalog const empty { ""sv };
    return empty;
}

::yugawara::schema::search_path const& sql_analyzer_options::schema_search_path() const noexcept {
    if (schema_search_path_) {
        return *schema_search_path_;
    }
    static ::yugawara::schema::search_path const empty {};
    return empty;
}

::yugawara::schema::declaration const& sql_analyzer_options::default_schema() const noexcept {
    if (default_schema_) {
        return *default_schema_;
    }
    using namespace std::string_view_literals;
    static ::yugawara::schema::declaration const empty { ""sv };
    return empty;
}

maybe_shared_ptr<::yugawara::schema::declaration const> sql_analyzer_options::default_schema_shared() const noexcept {
    auto result = default_schema_;
    if (result) {
        return result;
    }
    return maybe_shared_ptr { std::addressof(default_schema()) };
}

} // namespace mizugaki::analyzer
