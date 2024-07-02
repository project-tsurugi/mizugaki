#include <mizugaki/analyzer/details/analyze_type.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>
#include <takatori/type/octet.h>
#include <takatori/type/bit.h>
#include <takatori/type/date.h>
#include <takatori/type/time_of_day.h>
#include <takatori/type/time_point.h>

#include <takatori/util/fail.h>
#include <takatori/util/string_builder.h>

#include <mizugaki/ast/type/dispatch.h>

namespace mizugaki::analyzer::details {

namespace ttype = ::takatori::type;

using result_type = std::shared_ptr<ttype::data const>;

using ::takatori::util::fail;
using ::takatori::util::string_builder;

namespace {

class engine {
public:
    explicit engine(analyzer_context& context) :
        context_ { context }
    {}

    [[nodiscard]] result_type process(ast::type::type const& type) {
        return ast::type::dispatch(*this, type);
    }

    [[nodiscard]] result_type operator()(ast::type::type const& type) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported type: "
                        << type.node_kind()
                        << string_builder::to_string,
                type.region());
        return {};
    }

    [[nodiscard]] result_type operator()(ast::type::simple const& type) {
        using k = ast::type::kind;
        switch (type.node_kind()) {
            case k::unknown:
                return build(type, ttype::unknown {});
            case k::tiny_integer:
                return build(type, ttype::int1 {});
            case k::small_integer:
                return build(type, ttype::int2 {});
            case k::integer:
                return build(type, ttype::int4 {});
            case k::big_integer:
                return build(type, ttype::int8 {});
            case k::float_:
                // FIXME: configurable precision?
                return build(type, ttype::float4 {});
            case k::real:
                return build(type, ttype::float4 {});
            case k::double_precision:
                return build(type, ttype::float8 {});
            case k::boolean:
                return build(type, ttype::boolean {});
            case k::date:
                return build(type, ttype::date {});
            default:
                break;
        }
        fail();
    }

    [[nodiscard]] result_type operator()(ast::type::character_string const& type) {
        std::optional<ttype::character::size_type> size {};

        if (type.is_flexible_length()) {
            if (type.is_varying()) {
                size = std::nullopt;
            } else {
                context_.report(
                        sql_analyzer_code::flexible_length_is_not_supported,
                        string_builder {}
                                << "flexible length is not supported in char type"
                                << string_builder::to_string,
                        type.length().value().region());
                return {};
            }
        } else if (auto len = type.length()) {
            size = type.length().value().value();
            if (size == 0) {
                context_.report(
                        sql_analyzer_code::invalid_type_length,
                        "character type must not be empty",
                        type.length().value().region());
                return {};
            }
        } else if (!type.is_varying()) {
            size = 1;
        }
        return build(type, ttype::character {
                ttype::varying_t { type.is_varying() },
                size,
        });
    }

    [[nodiscard]] result_type operator()(ast::type::bit_string const& type) {
        std::optional<ttype::bit::size_type> size {};

        if (type.is_flexible_length()) {
            if (type.is_varying()) {
                size = std::nullopt;
            } else {
                context_.report(
                        sql_analyzer_code::flexible_length_is_not_supported,
                        string_builder {}
                                << "flexible length is not supported in char type"
                                << string_builder::to_string,
                        type.length().value().region());
                return {};
            }
        } else if (auto len = type.length()) {
            size = type.length().value().value();
            if (size == 0) {
                context_.report(
                        sql_analyzer_code::invalid_type_length,
                        "bit type must not be empty",
                        type.length().value().region());
                return {};
            }
        } else if (!type.is_varying()) {
            size = 1;
        }
        return build(type, ttype::bit {
                ttype::varying_t { type.is_varying() },
                size,
        });
    }

    [[nodiscard]] result_type operator()(ast::type::octet_string const& type) {
        std::optional<ttype::octet::size_type> size {};

        if (type.is_flexible_length()) {
            if (type.is_varying()) {
                size = std::nullopt;
            } else {
                context_.report(
                        sql_analyzer_code::flexible_length_is_not_supported,
                        string_builder {}
                                << "flexible length is not supported in char type"
                                << string_builder::to_string,
                        type.length().value().region());
                return {};
            }
        } else if (auto len = type.length()) {
            size = type.length().value().value();
            if (size == 0) {
                context_.report(
                        sql_analyzer_code::invalid_type_length,
                        "binary type must not be empty",
                        type.length().value().region());
                return {};
            }
        } else if (!type.is_varying()) {
            size = 1;
        }
        return build(type, ttype::octet {
                ttype::varying_t { type.is_varying() },
                size,
        });
    }

    [[nodiscard]] result_type operator()(ast::type::decimal const& type) {
        std::optional<ttype::decimal::size_type> precision {
                options().default_decimal_precision(),
        };
        std::optional<ttype::decimal::size_type> scale {
                0,
        };
        if (type.precision()) {
            if (type.is_flexible_precision()) {
                precision = std::nullopt;
            } else {
                precision = **type.precision();
            }
        }
        if (type.scale()) {
            if (type.is_flexible_scale()) {
                scale = std::nullopt;
            } else {
                scale = **type.scale();
            }
        }
        if (precision && precision == 0) {
            context_.report(
                    sql_analyzer_code::invalid_type_length,
                    "decimal type precision must not be zero",
                    type.precision().value().region());
            return {};
        }
        if (precision && precision > options().max_decimal_precision()) {
            context_.report(
                    sql_analyzer_code::invalid_type_length,
                    string_builder {}
                            << "too large decimal precision: " << precision.value()
                            << " (max precision is " << options().max_decimal_precision() << ")"
                            << string_builder::to_string,
                    type.precision()->region());
            return {};
        }
        if (scale && scale.value() > precision.value_or(options().max_decimal_precision())) {
            context_.report(
                    sql_analyzer_code::invalid_numeric_scale,
                    string_builder {}
                            << "too large scale value: " << scale.value()
                            << string_builder::to_string,
                    type.scale()->region());
            return {};
        }

        // NOTE: compiler allows to use `DECIMAL(*, *)`, but SQL engine may not support it.
        return build(type, ttype::decimal {
                precision,
                scale,
        });
    }

    [[nodiscard]] result_type operator()(ast::type::binary_numeric const& type) {
        using k = ast::type::kind;
        auto prec = type.precision();
        switch (type.node_kind()) {
            case k::binary_integer:
                if (type.is_flexible_precision() || !prec) {
                    return build(type, ttype::int8 {});
                }
                if (**prec == 0) {
                    context_.report(
                            sql_analyzer_code::invalid_type_length,
                            "integer type precision must not be zero",
                            type.precision().value().region());
                    return {};
                }
                if (**prec <= options().max_binary_integer1_precision()) {
                    return build(type, ttype::int1 {});
                }
                if (**prec <= options().max_binary_integer2_precision()) {
                    return build(type, ttype::int2 {});
                }
                if (**prec <= options().max_binary_integer4_precision()) {
                    return build(type, ttype::int4 {});
                }
                if (**prec <= options().max_binary_integer8_precision()) {
                    return build(type, ttype::int8 {});
                }
                context_.report(
                        sql_analyzer_code::invalid_type_length,
                        string_builder {}
                                << "too large integer precision: " << **prec
                                << " (max precision is " << options().max_binary_integer8_precision() << ")"
                                << string_builder::to_string,
                        prec->region());
                return {};

            case k::binary_float:
                if (type.is_flexible_precision() || !prec) {
                    return build(type, ttype::float8 {});
                }
                if (**prec == 0) {
                    context_.report(
                            sql_analyzer_code::invalid_type_length,
                            "float type precision must not be zero",
                            type.precision().value().region());
                    return {};
                }
                if (**prec <= options().max_binary_float4_precision()) {
                    return build(type, ttype::float4 {});
                }
                if (**prec <= options().max_binary_float8_precision()) {
                    return build(type, ttype::float8 {});
                }
                context_.report(
                        sql_analyzer_code::invalid_type_length,
                        string_builder {}
                                << "too large float precision: " << **prec
                                << " (max precision is " << options().max_binary_float8_precision() << ")"
                                << string_builder::to_string,
                        prec->region());
                return {};
            default:
                break;
        }
        fail();
    }

    [[nodiscard]] result_type operator()(ast::type::datetime const& type) {
        ::takatori::type::with_time_zone_t tz { type.has_time_zone() == true };
        using k = ast::type::kind;
        switch (type.node_kind()) {
            case k::time:
                return build(type, ttype::time_of_day { tz });
            case k::timestamp:
                return build(type, ttype::time_point { tz });
            default:
                break;
        }
        fail();
    }

    // FIXME: support interval qualifier
//    [[nodiscard]] result_type operator()(ast::type::interval const& type) {
//    }

    // FIXME: impl row type
//    [[nodiscard]] result_type operator()(ast::type::row const& type) {
//    }

    // FIXME: impl user defined type
    [[nodiscard]] result_type operator()(ast::type::user_defined const& type) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "user defined type is not supported: "
                        << type.name()->last_identifier()
                        << string_builder::to_string,
                type.region());
        return {};
    }

    // FIXME: impl collections type
//    [[nodiscard]] result_type operator()(ast::type::collection const& type) {
//    }

private:
    analyzer_context& context_;

    [[nodiscard]] analyzer_context::options_type const& options() const {
        return *context_.options();
    }
    
    template<class Type>
    [[nodiscard]] result_type build(ast::type::type const& source, Type&& type) {
        static_assert(std::is_rvalue_reference_v<decltype(type)>);
        (void) source;
        return context_.types().template get(std::forward<Type>(type));
    }
};

} // namespace

result_type analyze_type(
        analyzer_context& context,
        ast::type::type const& type) {
    engine e { context };
    return e.process(type);
}

} // namespace mizugaki::analyzer::details
