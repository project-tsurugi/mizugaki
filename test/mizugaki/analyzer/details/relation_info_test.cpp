#include <mizugaki/analyzer/details/relation_info.h>

#include <gtest/gtest.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class relation_info_test : public test_parent {};

TEST_F(relation_info_test, find_filtered_absent) {
    relation_info relation {};
    relation.add(column_info { {}, vdesc("v0"), "c0", });

    auto r = relation.find("c1", column_filter::all);
    EXPECT_TRUE(r.is_absent());
}

TEST_F(relation_info_test, find_filtered_found) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    relation_info relation {};
    relation.add(column_info { {}, v0, "c0", });
    relation.add(column_info { {}, v1, "c1", });

    auto r = relation.find("c1", column_filter::exported);
    ASSERT_TRUE(r.is_found());
    EXPECT_EQ(r->variable(), v1);
}

TEST_F(relation_info_test, find_filtered_not_exported) {
    auto v0 = vdesc("v0");
    relation_info relation {};
    relation.add(column_info { {}, v0, "c0", false, });

    EXPECT_TRUE(relation.find("c0", column_filter::exported).is_absent());

    auto r = relation.find("c0", column_filter::all);
    ASSERT_TRUE(r.is_found());
    EXPECT_EQ(r->variable(), v0);
}

TEST_F(relation_info_test, find_filtered_ambiguous) {
    relation_info relation {};
    relation.add(column_info { {}, vdesc("v0"), "c0", });
    relation.add(column_info { {}, vdesc("v1"), "c1", });
    relation.add(column_info { {}, vdesc("v2"), "c0", });

    auto r = relation.find("c0", column_filter::exported);
    EXPECT_TRUE(r.is_ambiguous());
}

TEST_F(relation_info_test, find_filtered_same_name_filtered) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    auto v2 = vdesc("v2");
    relation_info relation {};
    relation.add(column_info { {}, v0, "c0", false, });
    relation.add(column_info { {}, v1, "c0", });
    relation.add(column_info { {}, v2, "c0", false, });

    auto r = relation.find("c0", column_filter::exported);
    ASSERT_TRUE(r.is_found());
    EXPECT_EQ(r->variable(), v1);

    EXPECT_TRUE(relation.find("c0", column_filter::all).is_ambiguous());
}

TEST_F(relation_info_test, find_filtered_follows_flag_changes) {
    auto v0 = vdesc("v0");
    auto v1 = vdesc("v1");
    relation_info relation {};
    relation.add(column_info { {}, v0, "c0", });
    relation.add(column_info { {}, v1, "c0", false, });

    {
        auto r = relation.find("c0", column_filter::exported);
        ASSERT_TRUE(r.is_found());
        EXPECT_EQ(r->variable(), v0);
    }

    relation.columns()[0].exported() = false;
    EXPECT_TRUE(relation.find("c0", column_filter::exported).is_absent());

    relation.columns()[1].exported() = true;
    {
        auto r = relation.find("c0", column_filter::exported);
        ASSERT_TRUE(r.is_found());
        EXPECT_EQ(r->variable(), v1);
    }

    relation.columns()[0].exported() = true;
    EXPECT_TRUE(relation.find("c0", column_filter::exported).is_ambiguous());
}

TEST_F(relation_info_test, find_filtered_const) {
    auto v0 = vdesc("v0");
    relation_info relation {};
    relation.add(column_info { {}, v0, "c0", });
    relation.add(column_info { {}, vdesc("v1"), "c1", false, });

    auto const& r = relation;
    auto found = r.find("c0", column_filter::exported);
    ASSERT_TRUE(found.is_found());
    EXPECT_EQ(found->variable(), v0);
    EXPECT_TRUE(r.find("c1", column_filter::exported).is_absent());
}

} // namespace mizugaki::analyzer::details
