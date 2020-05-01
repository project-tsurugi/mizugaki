#include <mizugaki/document_map.h>

namespace mizugaki {

using ::takatori::util::optional_ptr;

document_map::document_map(document_map::entry_type const& entry)
    : document_map()
{
    add(entry);
}

document_map::document_map(std::shared_ptr<entry_type const> entry)
    : document_map()
{
    add(std::move(entry));
}

document_map& document_map::add(document_map::entry_type const& entry) {
    entry_map_.insert_or_assign(entry.location(), std::addressof(entry));
    return *this;
}

document_map& document_map::add(std::shared_ptr<entry_type const> entry) {
    entry_map_.insert_or_assign(entry->location(), std::move(entry));
    return *this;
}

optional_ptr<document_map::entry_type const> document_map::find(std::string_view name) const {
    if (auto it = entry_map_.find(name); it != entry_map_.end()) {
        auto&& entry = it->second;
        switch (entry.index()) {
            case naked_index: return *std::get<naked_index>(entry);
            case shared_index: return *std::get<shared_index>(entry);
            default: return {}; // broken?
        }
    }
    return {};
}

void document_map::reserve(std::size_t capacity) {
    entry_map_.reserve(capacity);
}

void document_map::clear() {
    entry_map_.clear();
}

} // namespace mizugaki
