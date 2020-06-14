#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/parser/sql_parser_result.h>

namespace mizugaki::parser {

class sql_driver {
public:
    using document_type = ::takatori::document::document;
    using result_type = sql_parser_result;

    using location_type = ast::node_region;

    explicit sql_driver(
            ::takatori::util::maybe_shared_ptr<document_type const> document,
            ::takatori::util::object_creator creator) noexcept :
        document_ { std::move(document) },
        creator_ { creator }
    {}

    [[nodiscard]] ::takatori::util::maybe_shared_ptr<document_type const> const& document() const noexcept;

    [[nodiscard]] ::takatori::util::object_creator get_object_creator() const noexcept;

    void add_comment(location_type location);

    [[nodiscard]] result_type& result() noexcept;

    [[nodiscard]] result_type const& result() const noexcept;

private:
    ::takatori::util::maybe_shared_ptr<document_type const> document_;
    ::takatori::util::object_creator creator_;

    result_type result_;
};

} // namespace sandbox
