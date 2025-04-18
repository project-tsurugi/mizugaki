#include "analyze_description.h"

#include <numeric>
#include <string_view>
#include <vector>

namespace mizugaki::analyzer::details {

namespace {

using std::string_view_literals::operator ""sv; // NOLINT(misc-unused-using-decls)

constexpr char line_separator = '\n';
constexpr std::string_view prefix_comment = "/**"sv;
constexpr std::string_view suffix_comment = "*/"sv;

[[nodiscard]] bool starts_with(std::string_view str, std::string_view prefix) {
    if (str.size() < prefix.size()) {
        return false;
    }
    return str.substr(0, prefix.size()) == prefix;
}

[[nodiscard]] bool ends_with(std::string_view str, std::string_view suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    return str.substr(str.size() - suffix.size()) == suffix;
}

[[nodiscard]] std::vector<std::string_view> split_lines(std::string_view str) {
    std::vector<std::string_view> lines;
    std::size_t start = 0;
    while (start < str.size()) {
        auto end = str.find('\n', start);
        if (end == std::string_view::npos) {
            lines.push_back(str.substr(start));
            break;
        }
        lines.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    return lines;
}

bool is_whitespace(char chr) {
    return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n';
}

void strip_first_line_head(std::string_view& str) {
    std::size_t offset = 0;
    if (offset < str.size() && str[offset] == '/') {
        ++offset;
    }
    while (offset < str.size() && str[offset] == '*') {
        ++offset;
    }
    if (offset > 0) {
        str.remove_prefix(offset);
    }
}

void strip_body_line_head(std::string_view& str) {
    std::size_t offset = 0;
    while (offset < str.size() && is_whitespace(str[offset])) {
        ++offset;
    }
    bool saw_asterisk = false;
    while (offset < str.size() && str[offset] == '*') {
        ++offset;
        saw_asterisk = true;
    }
    if (saw_asterisk) {
        str.remove_prefix(offset);
    }
}

constexpr char r_at(std::string_view str, std::size_t index) {
    return str[str.size() - 1 - index];
}

void strip_last_line_tail(std::string_view& str) {
    std::size_t offset = 0;
    while (offset < str.size() && r_at(str, offset) == '/') {
        ++offset;
    }
    while (offset < str.size() && r_at(str, offset) == '*') {
        ++offset;
    }
    while (offset < str.size() && is_whitespace(r_at(str, offset))) {
        ++offset;
    }
    if (offset > 0) {
        str.remove_suffix(offset);
    }
}

void strip_body_line_tail(std::string_view& str) {
    std::size_t offset = 0;
    while (offset < str.size() && is_whitespace(r_at(str, offset))) {
        ++offset;
    }
    if (offset > 0) {
        str.remove_suffix(offset);
    }
}

void strip_columns(std::string_view& str, bool is_first_line, bool is_last_line) {
    if (is_first_line) {
        strip_first_line_head(str);
    } else {
        strip_body_line_head(str);
    }
    if (is_last_line) {
        strip_last_line_tail(str);
    } else {
        strip_body_line_tail(str);
    }
}

void strip_leading_empty_lines(std::vector<std::string_view>& lines) {
    for (auto iter = lines.begin(); iter != lines.end(); ++iter) {
        if (!iter->empty()) {
            lines.erase(lines.begin(), iter);
            return;
        }
    }
    lines.clear();
}

void strip_trailing_empty_lines(std::vector<std::string_view>& lines) {
    while (!lines.empty()) {
        if (lines.back().empty()) {
            lines.pop_back();
        } else {
            break;
        }
    }
}

std::string_view extract_prefix_whitespaces(std::string_view str) {
    std::size_t offset = 0;
    while (offset < str.size() && is_whitespace(str[offset])) {
        ++offset;
    }
    return str.substr(0, offset);
}

std::string_view extract_common_prefix(std::string_view a, std::string_view b) {
    if (a.empty() || b.empty()) {
        return {};
    }
    std::size_t size = std::min(a.size(), b.size());
    for (std::size_t index = 0; index < size; ++index) {
        if (a[index] != b[index]) {
            return a.substr(0, index);
        }
    }
    return a.substr(0, size);
}

void strip_common_prefix_whitespaces(std::vector<std::string_view>& lines) {
    if (lines.empty()) {
        return;
    }
    auto prefix = extract_prefix_whitespaces(lines[0]);
    for (std::size_t index = 1; index < lines.size(); ++index) {
        auto next = lines[index];
        // ignore empty lines
        if (next.empty()) {
            continue;
        }
        prefix = extract_common_prefix(prefix, lines[index]);
    }

    for (auto& line : lines) {
        if (line.empty()) {
            continue;
        }
        line.remove_prefix(prefix.size());
    }
}

[[nodiscard]] std::string join_lines(std::vector<std::string_view> const& lines) {
    if (lines.empty()) {
        return {};
    }
    std::string result {};
    result.reserve(std::accumulate(
            lines.begin(),
            lines.end(),
            lines.size() - 1, // for line-break characters except the last line
            [](std::size_t acc, std::string_view line) -> std::size_t {
                return acc + line.size();
            }));

    for (std::size_t index = 0; index < lines.size(); ++index) {
        result.append(lines[index]);
        if (index != lines.size() - 1) {
            result.push_back(line_separator);
        }
    }
    return result;
}

[[nodiscard]] std::string process(std::string_view contents) {
    if (!starts_with(contents, prefix_comment) || !ends_with(contents, suffix_comment)) {
        return {};
    }
    auto lines = split_lines(contents);
    for (std::size_t index = 0; index < lines.size(); ++index) {
        auto& line = lines[index];
        strip_columns(line, index == 0, index == lines.size() - 1);
    }
    strip_leading_empty_lines(lines);
    strip_trailing_empty_lines(lines);
    strip_common_prefix_whitespaces(lines);

    auto result = join_lines(lines);
    return result;
}

} // namespace

std::string analyze_description(::takatori::document::document const& document, ast::node_region region) {
    if (!region) {
        return {};
    }
    if (document.size() < region.end) {
        return {};
    }
    auto contents = document.contents(region.begin, region.size());
    return process(contents);
}

std::string analyze_description(analyzer_context const& context, ast::node_region region) {
    if (auto document = context.source()) {
        return analyze_description(*document, region);
    }
    return {};
}

} // namespace mizugaki::analyzer::details
