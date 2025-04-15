#include <mizugaki/parser/sql_driver.h>

#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include <takatori/document/basic_document.h>

#include <mizugaki/ast/node_region.h>

namespace mizugaki::parser {

class sql_driver_test : public ::testing::Test {
public:
    sql_driver create(std::string source) {
        auto document = std::make_shared<::takatori::document::basic_document>("<input>", std::move(source));
        return sql_driver { document };
    }

    std::vector<std::string> contents(sql_driver const& driver, std::vector<ast::node_region> const& regions) {
        auto&& doc = driver.document().get();
        std::vector<std::string> result {};
        for (auto region : regions) {
            auto snippet = doc->contents(region.begin, region.end - region.begin);
            result.emplace_back(snippet);
        }
        return result;
    }

    ast::node_region add_comment(sql_driver& driver, std::size_t begin, std::size_t size) {
        ast::node_region result { begin, begin + size };
        driver.add_comment(result);
        return result;
    }

    ast::node_region add_separator(sql_driver& driver, std::size_t begin, std::size_t size) {
        ast::node_region result { begin, begin + size };
        driver.add_comment_separator(result);
        return result;
    }

    ast::node_region add_eof(sql_driver& driver) {
        auto&& doc = driver.document();
        return add_separator(driver, doc->size(), 0);
    }
};

TEST_F(sql_driver_test, add_comment) {
    std::string src = "()";
    // ----------------012
    auto driver = create(std::move(src));

    add_comment(driver, 0, 2);
    add_eof(driver);

    auto comments = contents(driver, driver.comments());
    ASSERT_EQ(comments.size(), 1);
    EXPECT_EQ(comments[0], "()");
}

TEST_F(sql_driver_test, add_comment_multiple) {
    std::string src = "(1)(2)(3)";
    // ----------------0123456789
    auto driver = create(std::move(src));

    add_comment(driver, 0, 3);
    add_comment(driver, 3, 3);
    add_comment(driver, 6, 3);
    add_eof(driver);

    auto comments = contents(driver, driver.comments());
    ASSERT_EQ(comments.size(), 3);
    EXPECT_EQ(comments[0], "(1)");
    EXPECT_EQ(comments[1], "(2)");
    EXPECT_EQ(comments[2], "(3)");
}

TEST_F(sql_driver_test, add_comment_whitespace) {
    std::string src = " (1) (2) ";
    // ----------------0123456789
    auto driver = create(std::move(src));

    add_comment(driver, 1, 3);
    add_comment(driver, 5, 3);
    add_eof(driver);

    auto comments = contents(driver, driver.comments());
    ASSERT_EQ(comments.size(), 2);
    EXPECT_EQ(comments[0], "(1)");
    EXPECT_EQ(comments[1], "(2)");
}

TEST_F(sql_driver_test, add_comment_leading) {
    std::string src = "(1)A";
    // ----------------01234
    auto driver = create(std::move(src));

    add_comment(driver, 0, 3);
    auto a_region = add_separator(driver, 3, 1);
    add_eof(driver);;

    auto a_leading = contents(driver, driver.leading_comments(a_region));
    ASSERT_EQ(a_leading.size(), 1);
    EXPECT_EQ(a_leading[0], "(1)");

    auto a_following = contents(driver, driver.following_comments(a_region));
    ASSERT_EQ(a_following.size(), 0);
}

TEST_F(sql_driver_test, add_comment_following) {
    std::string src = "A(1)";
    // ----------------01234
    auto driver = create(std::move(src));

    auto a_region = add_separator(driver, 0, 1);
    add_comment(driver, 1, 3);
    add_eof(driver);;

    auto a_leading = contents(driver, driver.leading_comments(a_region));
    ASSERT_EQ(a_leading.size(), 0);

    auto a_following = contents(driver, driver.following_comments(a_region));
    ASSERT_EQ(a_following.size(), 1);
    EXPECT_EQ(a_following[0], "(1)");
}

TEST_F(sql_driver_test, add_comment_leading_following) {
    std::string src = "(1)A(2)";
    // ----------------01234567
    auto driver = create(std::move(src));

    add_comment(driver, 0, 3);
    auto a_region = add_separator(driver, 3, 1);
    add_comment(driver, 4, 3);
    add_eof(driver);;

    auto a_leading = contents(driver, driver.leading_comments(a_region));
    ASSERT_EQ(a_leading.size(), 1);
    EXPECT_EQ(a_leading[0], "(1)");

    auto a_following = contents(driver, driver.following_comments(a_region));
    ASSERT_EQ(a_following.size(), 1);
    EXPECT_EQ(a_following[0], "(2)");
}

TEST_F(sql_driver_test, add_comment_separated) {
    std::string src = "A(1)BC(2)D";
    // ----------------01234567890
    auto driver = create(std::move(src));

    auto a_region = add_separator(driver, 0, 1);
    add_comment(driver, 1, 3);
    auto b_region = add_separator(driver, 4, 1);
    auto c_region = add_separator(driver, 5, 1);
    add_comment(driver, 6, 3);
    auto d_region = add_separator(driver, 9, 1);
    add_eof(driver);

    auto a_leading = contents(driver, driver.leading_comments(a_region));
    ASSERT_EQ(a_leading.size(), 0);

    auto a_following = contents(driver, driver.following_comments(a_region));
    ASSERT_EQ(a_following.size(), 1);
    EXPECT_EQ(a_following[0], "(1)");

    auto b_leading = contents(driver, driver.leading_comments(b_region));
    ASSERT_EQ(b_leading.size(), 1);
    EXPECT_EQ(b_leading[0], "(1)");

    auto b_following = contents(driver, driver.following_comments(b_region));
    ASSERT_EQ(b_following.size(), 0);

    auto c_leading = contents(driver, driver.leading_comments(c_region));
    ASSERT_EQ(c_leading.size(), 0);

    auto c_following = contents(driver, driver.following_comments(c_region));
    ASSERT_EQ(c_following.size(), 1);
    EXPECT_EQ(c_following[0], "(2)");

    auto d_leading = contents(driver, driver.leading_comments(d_region));
    ASSERT_EQ(d_leading.size(), 1);
    EXPECT_EQ(d_leading[0], "(2)");

    auto d_following = contents(driver, driver.following_comments(d_region));
    ASSERT_EQ(d_following.size(), 0);
}

TEST_F(sql_driver_test, leading_description_comment) {
    std::string src = "/**C*/A";
    // ----------------01234567
    auto driver = create(std::move(src));

    auto description = add_comment(driver, 0, 6);
    auto definition = add_separator(driver, 6, 1);
    add_eof(driver);

    auto result = driver.leading_description_comment(definition);
    EXPECT_EQ(result, description);
}

TEST_F(sql_driver_test, leading_description_comment_whiltespaces) {
    std::string src = "/**C*/\n A";
    // ----------------0123456 789
    auto driver = create(std::move(src));

    auto description = add_comment(driver, 0, 6);
    auto definition = add_separator(driver, 8, 1);
    add_eof(driver);

    auto result = driver.leading_description_comment(definition);
    EXPECT_EQ(result, description);
}

TEST_F(sql_driver_test, leading_description_comment_regular_comment) {
    std::string src = "/**C*//*X*/A";
    // ----------------0123456789012
    auto driver = create(std::move(src));

    auto description = add_comment(driver, 0, 6);
    add_comment(driver, 6, 5);
    auto definition = add_separator(driver, 11, 1);
    add_eof(driver);

    auto result = driver.leading_description_comment(definition);
    EXPECT_EQ(result, description);
}

TEST_F(sql_driver_test, leading_description_comment_multiple) {
    std::string src = "/**C1*//**C2*/A";
    // ----------------0123456789012345
    auto driver = create(std::move(src));

    add_comment(driver, 0, 7);
    auto description = add_comment(driver, 7, 7);
    auto definition = add_separator(driver, 14, 1);
    add_eof(driver);

    auto result = driver.leading_description_comment(definition);
    EXPECT_EQ(result, description);
}

TEST_F(sql_driver_test, leading_description_comment_not_description) {
    std::string src = "/*X*/A";
    // ----------------0123456
    auto driver = create(std::move(src));

    add_comment(driver, 0, 5);
    auto definition = add_separator(driver, 5, 1);
    add_eof(driver);

    auto result = driver.leading_description_comment(definition);
    EXPECT_FALSE(result);
}

TEST_F(sql_driver_test, leading_description_comment_missing) {
    std::string src = "/*X*/AB";
    // ----------------01234567
    auto driver = create(std::move(src));

    add_comment(driver, 0, 5);
    add_separator(driver, 5, 1);
    auto definition = add_separator(driver, 6, 1);
    add_eof(driver);

    auto result = driver.leading_description_comment(definition);
    EXPECT_FALSE(result);
}

TEST_F(sql_driver_test, leading_description_comment_disabled) {
    std::string src = "/**C*/A";
    // ----------------01234567
    auto driver = create(std::move(src));
    driver.enable_description_comments() = false;

    add_comment(driver, 0, 6);
    auto definition = add_separator(driver, 6, 1);
    add_eof(driver);

    auto result = driver.leading_description_comment(definition);
    EXPECT_FALSE(result);
}

} // namespace mizugaki::parser
