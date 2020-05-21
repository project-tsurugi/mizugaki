#include "type_translator.h"

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>

#include <takatori/util/string_builder.h>

#include <mizugaki/translator/util/type_dispatch.h>

namespace mizugaki::translator::details {

namespace type = ::takatori::type;

using translator_type = shakujo_translator::impl;
using result_type = std::shared_ptr<::takatori::type::data>;
using code_type = shakujo_translator_code;

using ::takatori::util::string_builder;

namespace {

class engine {
public:
    explicit engine(translator_type& translator) noexcept
        : translator_(translator)
    {}

    result_type operator()(::shakujo::model::type::Type const& node) {
        return report(code_type::unsupported_type, node, string_builder {}
                << node.kind());
    }

    result_type operator()(::shakujo::model::type::BooleanType const&) {
        return translator_.types().get(type::boolean {});
    }

    result_type operator()(::shakujo::model::type::Int32Type const&) {
        return translator_.types().get(type::int4 {});
    }

    result_type operator()(::shakujo::model::type::Int64Type const&) {
        return translator_.types().get(type::int8 {});
    }

    result_type operator()(::shakujo::model::type::Float32Type const&) {
        return translator_.types().get(type::float4 {});
    }

    result_type operator()(::shakujo::model::type::Float64Type const&) {
        return translator_.types().get(type::float8 {});
    }

    result_type operator()(::shakujo::model::type::CharType const& node) {
        return translator_.types().get(type::character {
                ~type::varying,
                node.size(),
        });
    }

    result_type operator()(::shakujo::model::type::VarCharType const& node) {
        return translator_.types().get(type::character {
                type::varying,
                node.size(),
        });
    }

    result_type operator()(::shakujo::model::type::StringType const&) {
        return translator_.types().get(type::character { type::varying });
    }

    result_type operator()(::shakujo::model::type::NullType const&) {
        return translator_.types().get(type::unknown {});
    }

private:
    translator_type& translator_;

    result_type report(code_type code, ::shakujo::model::Node const& node, string_builder&& builder) {
        translator_.diagnostics().emplace_back(
                code,
                builder << string_builder::to_string,
                translator_.region(node.region()));
        return {};
    }
};

} // namespace

type_translator::type_translator(translator_type& translator) noexcept
    : translator_(translator)
{}

result_type type_translator::process(::shakujo::model::type::Type const& node) {
    engine e { translator_ };
    return util::dispatch(e, node);
}

} // namespace mizugaki::translator::details
