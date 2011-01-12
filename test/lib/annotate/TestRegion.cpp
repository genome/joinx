#include "annotate/Region.hpp"

#include <string>
#include <stdexcept>
#include <gtest/gtest.h>

using namespace std;

TEST(TranscriptStructure, strands) {
    ASSERT_THROW(Region(0, 1, 2), runtime_error);
    ASSERT_THROW(Region(2, 1, 2), runtime_error);
    ASSERT_THROW(Region(-2, 1, 2), runtime_error);
    Region fwd(1, 5, 10);
    Region rev(-1, 5, 10);

    ASSERT_EQ( 1, fwd.strand());
    ASSERT_EQ( 5, fwd.start());
    ASSERT_EQ(10, fwd.stop());
    ASSERT_EQ( 5, fwd.strandedStart());
    ASSERT_EQ(10, fwd.strandedStop());

    ASSERT_EQ(-1, rev.strand());
    ASSERT_EQ( 5, rev.start());
    ASSERT_EQ(10, rev.stop());
    ASSERT_EQ(10, rev.strandedStart());
    ASSERT_EQ( 5, rev.strandedStop());

    typedef Region::RelativePos RelPos;

    RelPos pos = fwd.distance(4);
    ASSERT_EQ(RelPos::BEFORE, pos.type);
    ASSERT_EQ(1, pos.dist);
    for (int i = 5; i <= 10; ++i) {
        pos = fwd.distance(6);
        ASSERT_EQ(RelPos::IN, pos.type);
        ASSERT_EQ(0, pos.dist);
    }
    pos = fwd.distance(13);
    ASSERT_EQ(RelPos::AFTER, pos.type);
    ASSERT_EQ(3, pos.dist);

    pos = rev.distance(4);
    ASSERT_EQ(RelPos::AFTER, pos.type);
    ASSERT_EQ(1, pos.dist);
    for (int i = 5; i <= 10; ++i) {
        pos = rev.distance(6);
        ASSERT_EQ(RelPos::IN, pos.type);
        ASSERT_EQ(0, pos.dist);
    }
    pos = rev.distance(13);
    ASSERT_EQ(RelPos::BEFORE, pos.type);
    ASSERT_EQ(3, pos.dist);
}
