#include <mizugaki/placeholder_map.h>

namespace mizugaki {

using ::takatori::util::optional_ptr;

placeholder_map& placeholder_map::add(std::string name, entry_type entry) {
    entry_map_.insert_or_assign(std::move(name), std::move(entry));
    return *this;
}

optional_ptr<placeholder_map::entry_type const> placeholder_map::find(std::string_view name) const {
    if (auto it = entry_map_.find(name); it != entry_map_.end()) {
        return it->second;
    }
    return {};
}

void placeholder_map::reserve(std::size_t) {
    // do nothing
}

void placeholder_map::clear() {
    entry_map_.clear();
}

} // namespace mizugaki
