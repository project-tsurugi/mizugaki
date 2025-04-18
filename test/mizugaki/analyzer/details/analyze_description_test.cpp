#include <mizugaki/analyzer/details/analyze_description.h>

#include <gtest/gtest.h>

#include <string>

#include <takatori/document/basic_document.h>

namespace mizugaki::analyzer::details {

class analyze_description_test : public ::testing::Test {
public:
    [[nodiscard]] std::string process(std::string contents, bool strip_input = true) {
        if (strip_input) {
            contents = strip(std::move(contents));
        }
        ::takatori::document::basic_document doc { "<testing>", std::move(contents) };
        return analyze_description(doc, { 0, doc.size() });
    }

    [[nodiscard]] static std::string strip(std::string contents) {
        auto iter = std::find(contents.begin(), contents.end(), '/');
        if (iter != contents.end()) {
            contents.erase(contents.begin(), iter);
        }
        while (!contents.empty() && contents.back() != '/') {
            contents.pop_back();
        }
        return contents;
    }
};

TEST_F(analyze_description_test, simple) {
    std::string contents = R"(
        /**This is a test.*/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "This is a test.");
}

TEST_F(analyze_description_test, lines) {
    std::string contents = R"(
        /**
         * 1st.
         * 2nd.
         */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "1st.\n2nd.");
}

TEST_F(analyze_description_test, keep_leading_spaces) {
    std::string contents = R"(
/**
<pre>
{
    OK
}
</pre>
 */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "<pre>\n{\n    OK\n}\n</pre>");
}

TEST_F(analyze_description_test, remove_common_leading_spaces) {
    std::string contents = R"(
/**
    <pre>
    {
        OK
    }
    </pre>
 */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "<pre>\n{\n    OK\n}\n</pre>");
}

TEST_F(analyze_description_test, head_remove_leading_asterisks) {
    std::string contents = R"(
        /*********
         * B
         */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, body_remove_leading_whitespaces) {
    std::string contents = R"(
        /**
         B
         */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, body_remove_leading_asterisks) {
    std::string contents = R"(
        /**
         ******B
         */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, last_remove_trailing_asterisks) {
    std::string contents = R"(
        /**
         * B
         *******/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, body_keep_trailing_asterisks) {
    std::string contents = R"(
        /**
         * B *
         **/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B *");
}

TEST_F(analyze_description_test, body_remove_trailing_whitespaces) {
    std::string contents = "/**\nB  \n*/";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, single_remove_leading_asterisks) {
    std::string contents = R"(
        /***B*/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, single_remove_leading_whitespaces_after_asterisks) {
    std::string contents = R"(
        /*** *B*/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "*B");
}

TEST_F(analyze_description_test, single_remove_trailing_asterisks) {
    std::string contents = R"(
        /**B**/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, single_remove_trailing_whitespaces_before_asterisks) {
    std::string contents = R"(
        /**B* **/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B*");
}

TEST_F(analyze_description_test, single_empty_body) {
    std::string contents = R"(
        /***/
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "");
}

TEST_F(analyze_description_test, single_empty_content) {
    std::string contents = R"(
        /** */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "");
}

TEST_F(analyze_description_test, lines_remove_leading_empty_lines) {
    std::string contents = R"(
        /**
         *
         *
         * B
         */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, lines_remove_tailing_empty_lines) {
    std::string contents = R"(
        /**
         * B
         *
         *
         */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "B");
}

TEST_F(analyze_description_test, lines_keep_middle_empty_lines) {
    std::string contents = R"(
        /**
         * A
         *
         * B
         */
    )";
    auto result = process(contents);
    EXPECT_EQ(result, "A\n\nB");
}

TEST_F(analyze_description_test, crlf) {
    std::string contents = "/**head\r\ntail*/";
    auto result = process(contents);
    EXPECT_EQ(result, "head\ntail");
}

} // namespace mizugaki::analyzer::details
