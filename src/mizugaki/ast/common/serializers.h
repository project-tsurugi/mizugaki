#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <takatori/serializer/object_acceptor.h>
#include <takatori/serializer/json_printer.h>

#include <takatori/util/finalizer.h>

#include "mizugaki/ast/common/regioned.h"

/**
 * @brief provides helper functions of object_acceptor elements.
 */
namespace mizugaki::ast::common::serializers {

/**
 * @brief returns a block of `struct`.
 * @param acceptor the target acceptor
 * @return the finalizer of the block
 */
[[nodiscard]] ::takatori::util::finalizer struct_block(::takatori::serializer::object_acceptor& acceptor);

/**
 * @brief returns a block of `array`.
 * @param acceptor the target acceptor
 * @return the finalizer of the block
 */
[[nodiscard]] ::takatori::util::finalizer array_block(::takatori::serializer::object_acceptor& acceptor);

/**
 * @brief returns a block of `property`.
 * @param acceptor the target acceptor
 * @param name the property name
 * @return the finalizer of the block
 */
[[nodiscard]] ::takatori::util::finalizer property_block(
        ::takatori::serializer::object_acceptor& acceptor,
        std::string_view name);


template<class T>
void serialize(::takatori::serializer::object_acceptor& acceptor, T const& element);

/// @cond
namespace impl {

using ::takatori::serializer::object_acceptor;

template<class T, class = void>
struct element_serializer {
    void operator()(object_acceptor& acceptor, T const& element) {
        acceptor << element;
    }
};

template<class E, class D>
struct element_serializer<std::unique_ptr<E, D>> {
    void operator()(object_acceptor& acceptor, std::unique_ptr<E, D> const& element) {
        if (element) {
            serialize(acceptor, *element);
        }
    }
};

template<class E>
struct element_serializer<std::optional<E>> {
    void operator()(object_acceptor& acceptor, std::optional<E> const& element) {
        if (element) {
            serialize(acceptor, *element);
        }
    }
};

template<class E, class A>
struct element_serializer<std::vector<E, A>> {
    void operator()(object_acceptor& acceptor, std::vector<E, A> const& elements) {
        auto block = array_block(acceptor);
        for (auto&& e : elements) {
            serialize(acceptor, e);
        }
    }
};

template<class E>
struct element_serializer<regioned<E>> {
    void operator()(object_acceptor& acceptor, regioned<E> const& element) {
        serialize(acceptor, *element);
    }
};

template<>
struct element_serializer<bool> {
    void operator()(object_acceptor& acceptor, bool value) {
        acceptor.boolean(value);
    }
};

template<>
struct element_serializer<std::uint64_t> {
    void operator()(object_acceptor& acceptor, std::uint64_t value) {
        acceptor.unsigned_integer(value);
    }
};

template<>
struct element_serializer<std::string_view> {
    void operator()(object_acceptor& acceptor, std::string_view value) {
        acceptor.string(value);
    }
};

template<class C, class T, class A>
struct element_serializer<std::basic_string<C, T, A>> {
    void operator()(object_acceptor& acceptor, std::basic_string<C, T, A> const& element) {
        acceptor.string(element);
    }
};

template<class T>
struct element_serializer<T, std::enable_if_t<std::is_enum_v<T>>> {
    void operator()(object_acceptor& acceptor, T value) {
        acceptor.string(to_string_view(value));
    }
};

} // namespace impl
/// @endcond

/**
 * @brief serializes the given value.
 * @tparam T the value type
 * @param acceptor the target acceptor
 * @param element the target value
 */
template<class T>
void serialize(::takatori::serializer::object_acceptor& acceptor, T const& element) {
    impl::element_serializer<T> {}(acceptor, element);
}

/**
 * @brief prints the given value.
 * @tparam T the value type
 * @param out the target output
 * @param element the target value
 */
template<class T>
std::ostream& print(std::ostream& out, T const& element) {
    ::takatori::serializer::json_printer acceptor { out };
    impl::element_serializer<T> {}(acceptor, element);
    return out;
}

/**
 * @brief declare a property and serializes its property value.
 * @tparam T the value type
 * @param acceptor the target acceptor
 * @param name the property name
 * @param element the target value
 */
template<class T>
inline void property(
        ::takatori::serializer::object_acceptor& acceptor,
        std::string_view name,
        T const& element) {
    auto block = property_block(acceptor, name);
    serialize(acceptor, element);
}

/**
 * @brief serializes a region property of the element.
 * @tparam T the element type
 * @param acceptor the target acceptor
 * @param element the target value
 */
template<class T>
inline void region_property(
        ::takatori::serializer::object_acceptor& acceptor,
        T const& element) {
#if defined(SERIALIZE_REGION)
    using namespace std::string_view_literals;
    auto block = property_block(acceptor, "region"sv);
    serialize(acceptor, element.region());
#else
    (void) acceptor;
    (void) element;
#endif
}

/**
 * @brief creates a `struct` block for the given node.
 * @tparam T the node type
 * @param acceptor the target acceptor
 * @param node the node object
 * @return the `struct` block finalizer
 */
template<class T>
[[nodiscard]] inline ::takatori::util::finalizer struct_block(
        ::takatori::serializer::object_acceptor& acceptor,
        T const& node) {
    using namespace std::string_view_literals;
    auto block = struct_block(acceptor);
    property(acceptor, "node_kind"sv, node.node_kind());
    return block;
}

} // namespace mizugaki::ast::common::serializers
