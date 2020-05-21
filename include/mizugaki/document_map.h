#pragma once

#include <memory>
#include <unordered_map>
#include <variant>

#include <takatori/document/document.h>
#include <takatori/util/optional_ptr.h>

namespace mizugaki {

/**
 * @brief provides documents.
 */
class document_map {
public:
    /// @brief the entry type.
    using entry_type = ::takatori::document::document;

    /**
     * @brief creates a new empty document map.
     */
    document_map() noexcept = default;

    /**
     * @brief creates a new document map which holds the given element.
     * @param entry the first entry
     * @attention This only holds a reference of the document. Please keep the object ownership until dispose this map
     */
    document_map(entry_type const& entry); // NOLINT

    document_map(entry_type&& entry) = delete;
    document_map(entry_type const&& entry) = delete;

    /**
     * @brief creates a new document map which holds the given element.
     * @param entry the first entry
     */
    document_map(std::shared_ptr<entry_type const> entry); // NOLINT

    /**
     * @brief adds a view of document into this map.
     * @details This may overwrites the existing entry if there is document with the same location.
     * @param entry the entry
     * @return this
     * @attention This only holds a reference of the document. Please keep the object ownership until dispose this map
     */
    document_map& add(entry_type const& entry);

    document_map& add(entry_type&& entry) = delete;
    document_map& add(entry_type const&& entry) = delete;

    /**
     * @brief adds a view of document into this map.
     * @details This may overwrites the existing entry if there is document with the same location.
     * @param entry the entry
     * @return this
     */
    document_map& add(std::shared_ptr<entry_type const> entry);

    /**
     * @brief returns a previously added document entry.
     * @param name the document name
     * @return the corresponded document
     * @return empty if there is no such the document
     */
    [[nodiscard]] ::takatori::util::optional_ptr<entry_type const> find(std::string_view name) const;

    /**
     * @brief reserves capacity of document entries.
     * @param capacity the number of documents
     */
    void reserve(std::size_t capacity);

    /**
     * @brief removes all document entries.
     */
    void clear();

private:
    static constexpr std::size_t naked_index = 0;
    static constexpr std::size_t shared_index = 1;
    using pointer = std::variant<entry_type const*, std::shared_ptr<entry_type const>>;
    std::unordered_map<std::string_view, pointer> entry_map_;
};

} // namespace mizugaki

