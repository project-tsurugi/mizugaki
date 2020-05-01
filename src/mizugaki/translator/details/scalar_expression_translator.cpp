#include "scalar_expression_translator.h"

#include <variant>

#include <takatori/value/character.h>
#include <takatori/type/character.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>
#include <takatori/scalar/cast.h>
#include <takatori/scalar/unary.h>
#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/match.h>
#include <takatori/scalar/function_call.h>

#include <takatori/util/assertion.h>
#include <takatori/util/fail.h>
#include <takatori/util/downcast.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/sequence_view.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <yugawara/type/conversion.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

#include <mizugaki/translator/util/expression_dispatch.h>

#include "value_info_translator.h"
#include "type_info_translator.h"
#include "type_translator.h"

namespace mizugaki::translator::details {

namespace {

using translator_type = shakujo_translator::impl;
using result_type = ::takatori::util::unique_object_ptr<::takatori::scalar::expression>;
using diagnostic_type = shakujo_translator_diagnostic;
using code_type = diagnostic_type::code_type;

using ::takatori::util::fail;
using ::takatori::util::sequence_view;
using ::takatori::util::string_builder;
using ::takatori::util::unsafe_downcast;

namespace scalar = ::takatori::scalar;
namespace extension = ::yugawara::extension;

template<class T>
class complement {
public:
    constexpr complement(T value) noexcept : value_(value) {} // NOLINT
    constexpr T operator*() const noexcept { return value_; }

private:
    T value_;
};

class engine {
public:
    explicit engine(translator_type& translator, variable_scope const& scope) noexcept
        : translator_(translator)
        , scope_(scope)
    {}

    result_type process(::shakujo::model::expression::Expression const& node) {
        return util::dispatch(*this, node);
    }

    result_type operator()(::shakujo::model::expression::Expression const& node) {
        // FIXME: sub-query
        return report(code_type::unsupported_scalar_expression, node, string_builder {}
                << "must be a scalar expression: "
                << node.kind());
    }

    result_type operator()(::shakujo::model::expression::Literal const& node) {
        auto v = convert(*node.value(), node);
        if (!v) return {};
        
        auto t = convert(*node.type(), node);
        if (!t) return {};

        return create<scalar::immediate>(node, std::move(v), std::move(t));
    }

    result_type operator()(::shakujo::model::expression::VariableReference const& node) {
        auto v = scope_.find(*node.name());
        if (!v) {
            return report(code_type::variable_not_found, node, string_builder {}
                    << *node.name());
        }
        return create<scalar::variable_reference>(node, *v);
    }

    result_type operator()(::shakujo::model::expression::UnaryOperator const& node) {
        auto v = process(*node.operand());
        if (!v) return {};

        auto ks = convert(node);
        if (std::holds_alternative<scalar::unary_operator>(ks)) {
            auto k = std::get<scalar::unary_operator>(ks);
            return create<scalar::unary>(node, k, std::move(v));
        }
        if (std::holds_alternative<complement<scalar::unary_operator>>(ks)) {
            auto k = std::get<complement<scalar::unary_operator>>(ks);
            return create<scalar::unary>(
                    node,
                    scalar::unary_operator::conditional_not,
                    create<scalar::unary>(node, *k, std::move(v)));
        }
        return report(code_type::unsupported_scalar_expression, node, string_builder {}
                << "invalid unary operator: "
                << node.kind());
    }

    result_type operator()(::shakujo::model::expression::BinaryOperator const& node) {
        auto left = process(*node.left());
        if (!left) return {};

        auto right = process(*node.right());
        if (!right) return {};

        auto ks = convert(node);
        if (std::holds_alternative<scalar::binary_operator>(ks)) {
            auto k = std::get<scalar::binary_operator>(ks);
            return create<scalar::binary>(node, k, std::move(left), std::move(right));
        }
        if (std::holds_alternative<scalar::comparison_operator>(ks)) {
            auto k = std::get<scalar::comparison_operator>(ks);
            return create<scalar::compare>(node, k, std::move(left), std::move(right));
        }
        if (std::holds_alternative<scalar::match_operator>(ks)) {
            auto k = std::get<scalar::match_operator>(ks);
            return create<scalar::match>(node, k, std::move(left), std::move(right), char_string(""));
        }
        if (std::holds_alternative<complement<scalar::match_operator>>(ks)) {
            auto k = *std::get<complement<scalar::match_operator>>(ks);
            return create<scalar::unary>(
                    node,
                    scalar::unary_operator::conditional_not,
                    create<scalar::match>(node, k, std::move(left), std::move(right), char_string("")));
        }
        return report(code_type::unsupported_scalar_expression, node, string_builder {}
                << "invalid binary operator: "
                << node.kind());
    }

    result_type operator()(::shakujo::model::expression::TypeOperator const& node) {
        auto t = convert(*node.type());
        if (!t) return {};

        auto v = process(*node.operand());
        if (!v) return {};

        if (node.operator_kind() == ::shakujo::model::expression::TypeOperator::Kind::CAST) {
            return create<scalar::cast>(
                    node,
                    std::move(t),
                    scalar::cast::loss_policy_type::ignore,
                    std::move(v));
        }
        return report(code_type::unsupported_scalar_expression, node, string_builder {}
                << "invalid type operator: "
                << node.kind());
    }

    result_type operator()(::shakujo::model::expression::ImplicitCast const& node) {
        auto t = convert(*node.type(), node);
        if (!t) return {};

        auto v = process(*node.operand());
        if (!v) return {};

        return create<scalar::cast>(
                node,
                std::move(t),
                scalar::cast::loss_policy_type::ignore,
                std::move(v));
    }

    result_type operator()(::shakujo::model::expression::Placeholder const& node) {
        if (auto ph = translator_.placeholder(node.name())) {
            return ph;
        }
        return report(code_type::variable_not_found, node, string_builder {}
                << "unknown placeholder: "
                << node.name());
    }

    result_type operator()(::shakujo::model::expression::FunctionCall const& node) {
        auto name = as_simple_name(node.name());
        if (name.empty()) {
            return report(code_type::function_not_found, node, string_builder {} << *node.name());
        }

        ::takatori::util::reference_vector<scalar::expression> args { translator_.object_creator() };
        args.reserve(node.arguments().size());

        for (auto&& e : node.arguments()) {
            auto r = process(*e);
            if (!r) return {};
            args.push_back(std::move(r));
        }

        auto&& ts = translator_.type_buffer();
        ts.clear();
        ts.reserve(args.size());
        for (auto&& e : args) {
            auto&& analyzer = translator_.expression_analyzer();
            analyzer.clear_diagnostics();
            auto r = analyzer.resolve(e, false, translator_.types());
            if (analyzer.has_diagnostics()) {
                for (auto&& d : analyzer.diagnostics()) {
                    translator_.diagnostics().emplace_back(
                            code_type::type_error,
                            string_builder {}
                                    << d.code()
                                    << ": " << d.actual_type()
                                    << ", expected " << d.expected_categories()
                                    << string_builder::to_string,
                            d.region());
                }
                return {};
            }
            ts.emplace_back(std::move(r));
        }

        auto quantifier = convert(node.quantifier());
        using quanfifier_type = ::yugawara::aggregate::set_quantifier;
        if (!quantifier) {
            auto&& fs = collect_functions(name);
            auto&& as = collect_aggregates(name, quanfifier_type::all);
            translator_.type_buffer().clear();
            if (!fs.empty() && !as.empty()) {
                fs.clear();
                as.clear();
                return report(code_type::function_ambiguous, *node.name(), string_builder {}
                        << "'" << name << "' is defined as both scalar and aggregate function");
            }
            if (!fs.empty()) {
                auto r = resolve_overload(fs);
                if (!r) {
                    fs.clear();
                    return report(code_type::function_ambiguous, *node.name(), string_builder {}
                            << "function '" << name << "' is defined as both scalar and aggregate function");
                }
                auto desc = factory()(std::move(r));
                fs.clear();
                return create<scalar::function_call>(node, std::move(desc), std::move(args));
            }
            if (!as.empty()) {
                auto r = resolve_overload(as);
                if (!r) {
                    as.clear();
                    return report(code_type::function_ambiguous, *node.name(), string_builder {}
                            << "function '" << name << "' is defined as both scalar and aggregate function");
                }
                auto desc = factory()(std::move(r));
                as.clear();
                saw_aggregate_ = true;
                return create<extension::scalar::aggregate_function_call>(node, std::move(desc), std::move(args));
            }
        } else {
            auto&& as = collect_aggregates(name, *quantifier);
            translator_.type_buffer().clear();
            if (!as.empty()) {
                auto r = resolve_overload(as);
                if (!r) {
                    as.clear();
                    return report(code_type::function_ambiguous, *node.name(), string_builder {}
                            << "function '" << name << "' is defined as both scalar and aggregate function");
                }
                auto desc = factory()(std::move(r));
                as.clear();
                saw_aggregate_ = true;
                return create<extension::scalar::aggregate_function_call>(node, std::move(desc), std::move(args));
            }
        }
        return report(code_type::function_not_found, *node.name(), string_builder {}
                << "function '" << name << "' is not found");
    }

    [[nodiscard]] bool saw_aggregate() const noexcept {
        return saw_aggregate_;
    }

private:
    translator_type& translator_;
    variable_scope const& scope_;
    bool saw_aggregate_ { false };

    result_type report(code_type code, ::shakujo::model::Node const& node, string_builder&& builder) {
        translator_.diagnostics().emplace_back(
                code,
                builder << string_builder::to_string,
                translator_.region(node.region()));
        return {};
    }

    [[nodiscard]] ::yugawara::binding::factory factory() const noexcept {
        return ::yugawara::binding::factory { translator_.object_creator() };
    }

    template<class T, class... Args>
    result_type create(::shakujo::model::Node const& node, Args&&... args) {
        auto result = translator_.object_creator().create_unique<T>(std::forward<Args>(args)...);
        result->region() = translator_.region(node.region());
        return result;
    }

    result_type bless(::shakujo::model::Node const& node, result_type result) {
        result->region() = translator_.region(node.region());
        return result;
    }

    static std::string_view as_simple_name(::shakujo::model::name::Name const* name) {
        if (name != nullptr && name->kind() == ::shakujo::model::name::SimpleName::tag) {
            return unsafe_downcast<::shakujo::model::name::SimpleName>(*name).token();
        }
        return {};
    }

    value_info_translator::result_type convert(
            ::shakujo::common::core::Value const& info,
            ::shakujo::model::Node const& node) noexcept {
        value_info_translator e { translator_ };
        return e.process(info, node.region());
    }

    type_info_translator::result_type convert(
            ::shakujo::common::core::Type const& info,
            ::shakujo::model::Node const& node) noexcept {
        type_info_translator e { translator_ };
        return e.process(info, node.region());
    }

    type_info_translator::result_type convert(::shakujo::model::type::Type const& node) noexcept {
        type_translator e { translator_ };
        return e.process(node);
    }

    static std::variant<
            std::monostate,
            scalar::unary_operator,
            complement<scalar::unary_operator>>
    convert(::shakujo::model::expression::UnaryOperator const& node) {
        using from = ::shakujo::model::expression::UnaryOperator::Kind;
        using unary = scalar::unary_operator;
        switch (node.operator_kind()) {
            case from::PLUS: return unary::plus;
            case from::SIGN_INVERSION: return unary::sign_inversion;
            case from::CONDITIONAL_NOT: return unary::conditional_not;

            case from::IS_NULL: return unary::is_null;
            case from::IS_TRUE: return unary::is_true;
            case from::IS_FALSE: return unary::is_false;
            case from::IS_NOT_NULL: return complement { unary::is_null };
            case from::IS_NOT_TRUE: return complement { unary::is_true };
            case from::IS_NOT_FALSE: return complement { unary::is_false };

            default: return {};
        }
    }

    static std::variant<
            std::monostate,
            scalar::binary_operator,
            scalar::comparison_operator,
            scalar::match_operator,
            complement<scalar::match_operator>>
    convert(::shakujo::model::expression::BinaryOperator const& node) {
        using from = ::shakujo::model::expression::BinaryOperator::Kind;
        using binary = scalar::binary_operator;
        using compare = scalar::comparison_operator;
        using match = scalar::match_operator;
        switch (node.operator_kind()) {
            case from::ADD: return binary::add;
            case from::SUBTRACT: return binary::subtract;
            case from::MULTIPLY: return binary::multiply;
            case from::DIVIDE: return binary::divide;
            case from::REMAINDER: return binary::remainder;

            case from::EQUAL: return compare::equal;
            case from::NOT_EQUAL: return compare::not_equal;
            case from::LESS_THAN: return compare::less;
            case from::LESS_THAN_OR_EQUAL: return compare::less_equal;
            case from::GREATER_THAN: return compare::greater;
            case from::GREATER_THAN_OR_EQUAL: return compare::greater_equal;

            case from::CONDITIONAL_AND: return binary::conditional_and;
            case from::CONDITIONAL_OR: return binary::conditional_or;

            case from::CONCATENATION: return binary::concat;

            case from::LIKE: return match::like;
            case from::NOT_LIKE: return complement { match::like };

            default: return {};
        }
    }

    static std::optional<::yugawara::aggregate::set_quantifier> convert(
            ::shakujo::model::expression::FunctionCall::Quantifier quantifier) noexcept {
        using from = ::shakujo::model::expression::FunctionCall::Quantifier;
        using to = ::yugawara::aggregate::set_quantifier;
        switch (quantifier) {
            case from::ABSENT:
                return std::nullopt;
            case from::ALL:
            case from::ASTERISK:
                return to::all;
            case from::DISTINCT:
                return to::distinct;
        }
        fail();
    }

    result_type char_string(std::string_view str) {
        auto c = translator_.object_creator();
        return c.create_unique<scalar::immediate>(
                translator_.values().get(::takatori::value::character { str, translator_.object_creator().allocator() }),
                translator_.types().get(::takatori::type::character { str.size() }));
    }

    [[nodiscard]] std::vector<std::shared_ptr<::yugawara::function::declaration const>>& collect_functions(
            std::string_view name) {
        // NOTE: should be filled to translator_.type_buffer()
        translator_.function_buffer().clear();
        auto param_count = translator_.type_buffer().size();
        translator_.context().functions().each(
                name,
                param_count,
                [this](std::shared_ptr<::yugawara::function::declaration const> const& d) {
                    if (is_callable(d->shared_parameter_types(), translator_.type_buffer())) {
                        translator_.function_buffer().emplace_back(d);
                    }
                });
        return translator_.function_buffer();
    }

    [[nodiscard]] std::vector<std::shared_ptr<::yugawara::aggregate::declaration const>>& collect_aggregates(
            std::string_view name,
            ::yugawara::aggregate::set_quantifier quantifier) {
        // NOTE: should be filled to translator_.type_buffer()
        translator_.aggregate_buffer().clear();
        translator_.context().aggregates().each(
                name,
                quantifier,
                translator_.type_buffer().size(),
                [this](std::shared_ptr<::yugawara::aggregate::declaration const> const& d) {
                    if (is_callable(d->shared_parameter_types(), translator_.type_buffer())) {
                        translator_.aggregate_buffer().emplace_back(d);
                    }
                });
        return translator_.aggregate_buffer();
    }

    [[nodiscard]] static bool is_callable(
            sequence_view<std::shared_ptr<::takatori::type::data const> const> params,
            std::vector<std::shared_ptr<::takatori::type::data const>> const& args) {
        BOOST_ASSERT(params.size() == args.size()); // NOLINT
        for (std::size_t i = 0, n = params.size(); i < n; ++i) {
            auto r = ::yugawara::type::is_assignment_convertible(*args[i], *params[i]);
            if (r != true) {
                return false;
            }
        }
        return true;
    }

    template<class T>
    [[nodiscard]] static std::shared_ptr<T const> resolve_overload(
            std::vector<std::shared_ptr<T const>> const& candidates) {
        if (candidates.empty()) {
            return {};
        }
        std::size_t result_at = 0;
        for (std::size_t i = 1, n = candidates.size(); i < n; ++i) {
            auto const* left = candidates[result_at].get();
            auto const* right = candidates[i].get();
            auto lw = left->has_wider_parameters(*right);
            auto rw = right->has_wider_parameters(*left);
            if (lw == rw) {
                return {};
            }
            if (lw) {
                result_at = i;
            }
        }
        return candidates[result_at];
    }
};

} // namespace

scalar_expression_translator::scalar_expression_translator(translator_type& translator) noexcept
    : translator_(translator)
{}

result_type scalar_expression_translator::process(
        shakujo::model::expression::Expression const& node,
        variable_scope const& scope) {
    engine e { translator_, scope };
    auto result = e.process(node);
    saw_aggregate_ = e.saw_aggregate();
    return result;
}

bool scalar_expression_translator::saw_aggregate() const noexcept {
    return saw_aggregate_;
}

} // namespace mizugaki::translator::details
