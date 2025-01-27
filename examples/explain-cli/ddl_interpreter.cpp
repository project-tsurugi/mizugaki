#include "ddl_interpreter.h"

#include <takatori/statement/dispatch.h>

#include <takatori/util/downcast.h>

#include <yugawara/storage/configurable_provider.h>

#include <yugawara/binding/extract.h>
#include <yugawara/binding/schema_info.h>

namespace mizugaki::examples::explain_cli {

using ::takatori::util::downcast;

namespace {

[[nodiscard]] ::yugawara::storage::configurable_provider&
extract_storage_provider(::yugawara::schema::declaration const& schema) {
    return downcast<::yugawara::storage::configurable_provider>(schema.storage_provider());
}

class engine {
public:
    void operator()(::takatori::statement::statement const&) const noexcept {
        // default - do nothing
    }

    void operator()(::takatori::statement::create_table const& statement) const {
        auto&& schema = ::yugawara::binding::extract(statement.schema());
        auto&& storage = extract_storage_provider(schema);
        auto&& table = ::yugawara::binding::extract_shared(statement.definition());
        auto&& primary_index = ::yugawara::binding::extract_shared<::yugawara::storage::index>(statement.primary_key());

        storage.add_table(std::move(table));
        storage.add_index(primary_index.ownership());
    }

    void operator()(::takatori::statement::create_index const& statement) const {
        auto&& schema = ::yugawara::binding::extract(statement.schema());
        auto&& storage = extract_storage_provider(schema);
        auto&& index = ::yugawara::binding::extract_shared<::yugawara::storage::index>(statement.definition());

        storage.add_index(index.ownership());
    }

    void operator()(::takatori::statement::drop_table const& statement) const {
        auto&& schema = ::yugawara::binding::extract(statement.schema());
        auto&& storage = extract_storage_provider(schema);

        auto&& table = ::yugawara::binding::extract(statement.target());
        if (auto primary_index = storage.find_primary_index(table)) {
            storage.remove_index(primary_index->simple_name());
        }
        storage.remove_relation(table.simple_name());
    }

    void operator()(::takatori::statement::drop_index const& statement) const {
        auto&& schema = ::yugawara::binding::extract(statement.schema());
        auto&& storage = extract_storage_provider(schema);

        auto&& index = ::yugawara::binding::extract<::yugawara::storage::index>(statement.target());
        storage.remove_index(index.simple_name());
    }
};

} // namespace

void interpret(::takatori::statement::statement const& statement) {
    engine e {};
    takatori::statement::dispatch(e, statement);
}

} // namespace mizugaki::examples::explain_cli
