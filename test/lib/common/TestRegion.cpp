#include "common/Region.hpp"

#include <gtest/gtest.h>

using namespace std;

TEST(TestRegion, overlapPartial) {
    Region x(10, 20);
    Region y(9, 12);
    EXPECT_EQ(2, x.overlap(y));
    EXPECT_EQ(2, y.overlap(x));
}

TEST(TestRegion, overlapContained) {
    Region x(10, 20);
    Region y(15, 18);
    EXPECT_EQ(3, x.overlap(y));
    EXPECT_EQ(3, y.overlap(x));
}

TEST(TestRegion, overlapAdjacent) {
    Region x(10, 20);
    Region y(20, 30);
    EXPECT_EQ(0, x.overlap(y));
    EXPECT_EQ(0, y.overlap(x));
}

TEST(TestRegion, overlapDisjoint) {
    Region x(10, 20);
    Region y(90, 120);
    EXPECT_EQ(0, x.overlap(y));
    EXPECT_EQ(0, y.overlap(x));
}

TEST(TestRegion, size) {
    EXPECT_EQ(0, (Region(10, 10)).size());
    EXPECT_EQ(1, (Region(10, 11)).size());
    EXPECT_EQ(11, (Region(10, 21)).size());
}

TEST(TestRegion, contains) {
    Region outer(10, 20);
    for (int i = 10; i < 20; ++i) {
        for (int j = i; j < 20; ++j) {
            Region inner(i, j);
            EXPECT_TRUE(outer.contains(inner));
            EXPECT_TRUE(inner.isContainedIn(outer));
        }
    }
}
