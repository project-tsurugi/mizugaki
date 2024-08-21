#include <mizugaki/parser/sql_parser_element_kind.h>

#include <gtest/gtest.h>

namespace mizugaki::parser {

class sql_parser_element_kind_test : public ::testing::Test {};

using kind_type = sql_parser_element_kind;

TEST_F(sql_parser_element_kind_test, map) {
    sql_parser_element_map<int> map {};
    EXPECT_EQ(map.get<kind_type::statement>(), 0);

    map.get<kind_type::statement>() = 100;
    EXPECT_EQ(map[kind_type::statement], 100);

    sql_parser_element_map<int> const& cmap = map;
    map.get<kind_type::string_literal_concatenation>() = 100;
    EXPECT_EQ(cmap[kind_type::string_literal_concatenation], 100);
}

TEST_F(sql_parser_element_kind_test, find) {
    EXPECT_EQ(find_sql_parser_element_kind("statement"), kind_type::statement);
    EXPECT_EQ(find_sql_parser_element_kind("string_literal_concatenation"), kind_type::string_literal_concatenation);
    EXPECT_EQ(find_sql_parser_element_kind("select_element"), kind_type::select_element);
    EXPECT_EQ(find_sql_parser_element_kind("MISSING"), std::nullopt);
}

TEST_F(sql_parser_element_kind_test, output) {
    sql_parser_element_map<int> map {};
    for (auto index = static_cast<std::size_t>(sql_parser_element_kind_first);
            index <= static_cast<std::size_t>(sql_parser_element_kind_last);
            ++index) {
        auto kind = static_cast<kind_type>(index);
        map[kind] = index + 1;
    }
    std::cout << map << std::endl;
}

} // namespace mizugaki::parser
