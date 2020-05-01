#pragma once

#include <map>
#include <string>
#include <string_view>

#include <takatori/util/optional_ptr.h>

#include "placeholder_entry.h"

namespace mizugaki {

/**
 * @brief provides placeholders.
 */
class placeholder_map {
public:
    /// @brief the entry type.
    using entry_type = placeholder_entry;

    /**
     * @brief adds a placeholder into this map.
     * @details This may overwrites the existing entry if there is placeholder with the same name.
     * @param name the placeholder name - ordinary starts with `":"`.
     * @param entry the entry
     * @return this
     */
    placeholder_map& add(std::string name, entry_type entry);

    /**
     * @brief returns a previously added placeholder entry.
     * @param name the placeholder name
     * @return the corresponded placeholder
     * @return empty if there is no such the placeholder
     */
    [[nodiscard]] ::takatori::util::optional_ptr<entry_type const> find(std::string_view name) const;

    /**
     * @brief reserves capacity of placeholder entries.
     * @param capacity the number of placeholders
     */
    void reserve(std::size_t capacity);

    /**
     * @brief removes all placeholder entries.
     */
    void clear();

private:
    std::map<std::string, entry_type, std::less<>> entry_map_;
};

} // namespace mizugaki

