#include <mizugaki/ast/node_region.h>

#include <gtest/gtest.h>

#include <takatori/util/hash.h>

namespace mizugaki::ast {

using ::takatori::util::hash;

class node_region_test : public ::testing::Test {};

namespace {

class test_regioned {
public:
    [[nodiscard]] node_region& region() noexcept {
        return region_;
    }

    [[nodiscard]] node_region const& region() const noexcept {
        return region_;
    }

private:
    node_region region_;
};

} // namespace

static_assert(std::is_default_constructible_v<node_region>);
static_assert(std::is_constructible_v<node_region, std::size_t, std::size_t>);
static_assert(std::is_trivially_copyable_v<node_region>);
static_assert(std::is_trivially_destructible_v<node_region>);

static_assert(has_region_v<test_regioned>);
static_assert(!has_region_v<int>);
static_assert(!has_region_v<std::string>);

TEST_F(node_region_test, compare) {
    node_region a { 10, 30 };
    node_region b { 10, 30 };
    node_region c { 10, 20 };
    node_region d { 20, 30 };

    EXPECT_EQ(a, b);
    EXPECT_NE(b, c);
    EXPECT_NE(b, d);
    EXPECT_NE(c, d);
    
    EXPECT_EQ(hash(a), hash(b));
    EXPECT_NE(hash(b), hash(c));
    EXPECT_NE(hash(b), hash(d));
    EXPECT_NE(hash(c), hash(d));
}

TEST_F(node_region_test, operator_or) {
    node_region a { 10, 30 };
    node_region b { 20, 40 };
    EXPECT_EQ(a | b, node_region(10, 40));
}

TEST_F(node_region_test, output) {
    node_region r { 10, 30 };
    std::cout << r << std::endl;
}

} // namespace mizugaki::ast
