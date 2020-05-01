#pragma once

#include <iostream>
#include <memory>
#include <utility>

#include <takatori/type/data.h>
#include <takatori/value/data.h>

#include <takatori/scalar/expression.h>

#include <takatori/util/object_creator.h>

namespace mizugaki {

/**
 * @brief provides a scalar expression lazily.
 */
class placeholder_entry {
public:
    /**
     * @brief creates a new instance.
     * @param value the value
     * @param type the value type
     */
    placeholder_entry(
            std::shared_ptr<::takatori::value::data const> value,
            std::shared_ptr<::takatori::type::data const> type) noexcept;

    /**
     * @brief creates a new instance.
     * @param type the value type
     * @param value the value
     */
    placeholder_entry(
            std::shared_ptr<::takatori::type::data const> type,
            std::shared_ptr<::takatori::value::data const> value) noexcept;

    /**
     * @brief creates a new instance.
     * @param value the value
     * @param type the value type
     * @attention this will take a copy of arguments
     */
    placeholder_entry(::takatori::value::data&& value, ::takatori::type::data&& type);

    /**
     * @brief creates a new instance.
     * @param value the value
     * @param type the value type
     * @attention this will take a copy of arguments
     */
    placeholder_entry(::takatori::type::data&& type, ::takatori::value::data&& value);

    /**
     * @brief resolves this placeholder.
     * @param creator the object creator for the resulting scalar expression
     * @return the created scalar expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<::takatori::scalar::expression> resolve(
            ::takatori::util::object_creator creator = {}) const;

    /**
     * @brief appends string representation of the given value.
     * @param out the target output
     * @param value the target value
     * @return the output
     */
    friend std::ostream& operator<<(std::ostream& out, placeholder_entry const& value);

private:
    std::shared_ptr<::takatori::value::data const> value_;
    std::shared_ptr<::takatori::type::data const> type_;
};

} // namespace mizugaki

