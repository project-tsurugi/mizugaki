#pragma once

#include <memory>
#include <variant>
#include <vector>

#include <takatori/relation/graph.h>
#include <takatori/statement/statement.h>
#include <takatori/util/object_creator.h>

#include "shakujo_translator_result_kind.h"
#include "shakujo_translator_diagnostic.h"

namespace mizugaki::translator {

/**
 * @brief unwrap only if the target type is std::unique_ptr.
 * @tparam T the target type
 */
template<class T, class = void>
struct unwrap_if_unique_ptr {
    /// @brief the target type.
    using type = T;

    /// @brief the element reference type.
    using reference = std::add_lvalue_reference_t<type>;

    /// @brief the const element reference type.
    using const_reference = std::add_lvalue_reference_t<std::add_const_t<type>>;

    /**
     * @brief returns the unwrapped element.
     * @param value the wrapped element
     * @return the unwrapped element
     */
    T& operator()(T& value) noexcept {
        return value;
    }

    /// @copydoc operator()()
    T const& operator()(T const& value) noexcept {
        return value;
    }
};

/**
 * @brief unwrap only if the target type is std::unique_ptr.
 * @tparam T the target type
 */
template<class T>
struct unwrap_if_unique_ptr<T, std::enable_if_t<std::is_trivially_copyable_v<T>>> {
    /// @brief the target type.
    using type = T;

    /// @brief the element reference type.
    using reference = type;

    /// @brief the const element reference type.
    using const_reference = type;

    /**
     * @brief returns the unwrapped element.
     * @param value the wrapped element
     * @return the unwrapped element
     */
    T& operator()(T& value) noexcept {
        return value;
    }

    /// @copydoc operator()()
    T const& operator()(T const& value) noexcept {
        return value;
    }
};

/**
 * @brief unwrap only if the target type is std::unique_ptr.
 * @tparam E the element type
 * @tparam D the deleter type
 */
template<class E, class D>
struct unwrap_if_unique_ptr<std::unique_ptr<E, D>> {
    /// @brief the target type.
    using type = std::unique_ptr<E, D>;

    /// @brief the element reference type.
    using reference = std::add_lvalue_reference_t<E>;

    /// @brief the const element reference type.
    using const_reference = std::add_lvalue_reference_t<std::add_const_t<E>>;

    /**
     * @brief returns the unwrapped element.
     * @param value the wrapped element
     * @return the unwrapped element
     */
    E& operator()(std::unique_ptr<E, D>& value) noexcept {
        return *value;
    }

    /// @copydoc operator()()
    E const& operator()(std::unique_ptr<E, D> const& value) noexcept {
        return *value;
    }
};

/**
 * @brief an operation result of shakujo_translator.
 */
class shakujo_translator_result {
public:
    /// @brief the element kind.
    using kind_type = shakujo_translator_result_kind;

    /// @brief the entity type
    using entity_type = std::variant<
            std::vector<shakujo_translator_diagnostic>, // diagnostics
            ::takatori::util::unique_object_ptr<::takatori::relation::graph_type>, // execution_plan
            ::takatori::util::unique_object_ptr<::takatori::statement::statement>>; // statement

    /**
     * @brief the element type for each kind.
     * @tparam Kind the element kind
     */
    template<kind_type Kind>
    using element_type = std::variant_alternative_t<
            static_cast<std::size_t>(Kind),
            entity_type>;

    /**
     * @brief creates a new invalid instance.
     */
    constexpr shakujo_translator_result() noexcept = default;

    /**
     * @brief creates a new instance.
     * @param element the diagnostics
     */
    shakujo_translator_result(std::vector<shakujo_translator_diagnostic> element) noexcept; // NOLINT

    /**
     * @brief creates a new instance.
     * @param element the holding element
     */
    shakujo_translator_result(::takatori::util::unique_object_ptr<::takatori::relation::graph_type> element) noexcept; // NOLINT

    /**
     * @brief creates a new instance.
     * @param element the holding element
     */
    shakujo_translator_result(::takatori::util::unique_object_ptr<::takatori::statement::statement> element) noexcept; // NOLINT

    /**
     * @brief returns the element kind.
     * @return the element kind
     */
    [[nodiscard]] constexpr kind_type kind() const noexcept {
        auto index = entity_.index();
        if (index == std::variant_npos) {
            return kind_type::diagnostics;
        }
        return static_cast<kind_type>(index);
    }

    /**
     * @brief returns whether or not this is valid result.
     * @return true if this is a valid result
     * @return false otherwise
     */
    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return kind() != kind_type::diagnostics;
    }

    /// @copydoc is_valid()
    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return is_valid();
    }

    /**
     * @brief returns the holding element.
     * @tparam Kind the element kind
     * @return the holding element
     * @throws std::bad_variant_access if the holding element kind is inconsistent
     */
    template<kind_type Kind>
    [[nodiscard]] typename unwrap_if_unique_ptr<element_type<Kind>>::reference element() {
        if constexpr (Kind == kind_type::diagnostics) { // NOLINT
            if (entity_.index() == std::variant_npos) {
                return {};
            }
        }
        unwrap_if_unique_ptr<element_type<Kind>> unwrap;
        return unwrap(std::get<static_cast<std::size_t>(Kind)>(entity_));
    }

    /// @copydoc element()
    template<kind_type Kind>
    [[nodiscard]] typename unwrap_if_unique_ptr<element_type<Kind>>::const_reference element() const {
        if constexpr (Kind == kind_type::diagnostics) { // NOLINT
            if (entity_.index() == std::variant_npos) {
                return {};
            }
        }
        unwrap_if_unique_ptr<element_type<Kind>> unwrap;
        return unwrap(std::get<static_cast<std::size_t>(Kind)>(entity_));
    }

    /**
     * @brief releases the holding element.
     * @tparam Kind the element kind
     * @return the holding element
     * @throws std::bad_variant_access if the holding element kind is inconsistent
     */
    template<kind_type Kind>
    [[nodiscard]] typename unwrap_if_unique_ptr<element_type<Kind>>::type release() {
        if constexpr (Kind == kind_type::diagnostics) { // NOLINT
            if (entity_.index() == std::variant_npos) {
                return {};
            }
        }
        return std::move(std::get<static_cast<std::size_t>(Kind)>(entity_));
    }

    /**
     * @brief appends string representation of the given value.
     * @param out the target output
     * @param value the target value
2     * @return the output stream
     */
    friend std::ostream& operator<<(std::ostream& out, shakujo_translator_result const& value);

private:
    entity_type entity_;
};

} // namespace mizugaki::translator
