#include "common/LocusCompare.hpp"

#include <gtest/gtest.h>
#include <string>

namespace {
    struct TestObject {
        std::string chrom_;
        int start_;
        int stop_;

        std::string const& chrom() const { return chrom_; }
        int start() const { return start_; }
        int stop() const { return stop_; }

        int startWithoutPadding() const { return -start_; }
        int stopWithoutPadding() const { return -stop_; }
    };
}

class TestLocusCompare : public ::testing::Test {
public:

    void SetUp() {
        // these are in sorted order
        objs = {
              { "1", 10, 20}
            , { "1", 10, 21}
            , { "1", 11, 15}
            , { "9", 20, 30}
            , {"10", 10, 13}
            , {"10", 11, 12}
            };
    }

protected:
    std::vector<TestObject> objs;
};

TEST_F(TestLocusCompare, cmp_default) {
    LocusCompare<> lcmp;
    EXPECT_EQ(0, lcmp(objs[0], objs[0]));
    for (std::size_t i = 1; i < objs.size(); ++i) {
        EXPECT_EQ(0, lcmp(objs[i], objs[i]));
        EXPECT_LT(lcmp(objs[i - 1], objs[i]), 0);
        EXPECT_GT(lcmp(objs[i], objs[i - 1]), 0);
    }
}

TEST_F(TestLocusCompare, cmp_without_padding) {
    LocusCompare<UnpaddedCoordinateView> lcmp;


    for (std::size_t i = 0; i < objs.size(); ++i) {
        EXPECT_EQ(0, lcmp(objs[i], objs[i]));
    }

    // chr1
    for (std::size_t i = 1; i < 3; ++i) {
        EXPECT_EQ(0, lcmp(objs[i], objs[i]));
        EXPECT_GT(lcmp(objs[i - 1], objs[i]), 0);
        EXPECT_LT(lcmp(objs[i], objs[i - 1]), 0);
    }

    // chr1 vs chr9
    EXPECT_LT(lcmp(objs[2], objs[3]), 0);
    EXPECT_GT(lcmp(objs[3], objs[2]), 0);

    // chr9 vs chr10
    EXPECT_LT(lcmp(objs[3], objs[4]), 0);
    EXPECT_GT(lcmp(objs[4], objs[3]), 0);

    // chr10
    EXPECT_GT(lcmp(objs[4], objs[5]), 0);
    EXPECT_LT(lcmp(objs[5], objs[4]), 0);
}
