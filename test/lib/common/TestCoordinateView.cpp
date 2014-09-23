#include "common/CoordinateView.hpp"

#include <gtest/gtest.h>

#include <string>

struct TestObject {
    std::string chrom_;
    int64_t start_;
    int64_t stop_;

    std::string const& chrom() const { return chrom_; }
    int64_t start() const { return start_; }
    int64_t stop() const { return stop_; }

    int64_t startWithoutPadding() const { return -start_; }
    int64_t stopWithoutPadding() const { return -stop_; }

};

TEST(TestCoordinateView, default_coordinate_view) {
    DefaultCoordinateView view;
    TestObject obj{"X", 10, 20};
    EXPECT_EQ("X", view.chrom(obj));
    EXPECT_EQ(&view.chrom(obj), &obj.chrom_);
    EXPECT_EQ(10, view.start(obj));
    EXPECT_EQ(20, view.stop(obj));
}


TEST(TestCoordinateView, unpadded_coordinate_view) {
    UnpaddedCoordinateView view;
    TestObject obj{"X", 10, 20};
    EXPECT_EQ("X", view.chrom(obj));
    EXPECT_EQ(&view.chrom(obj), &obj.chrom_);
    EXPECT_EQ(-10, view.start(obj));
    EXPECT_EQ(-20, view.stop(obj));
}
