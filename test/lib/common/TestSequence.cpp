#include "common/Sequence.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

using namespace std;

TEST(Sequence, reverseComplement) {
    string input = "acgtrymkswhbvdnx";
    string expected = "XNHBVDWSMKRYACGT";

    Sequence seq(input);
    ASSERT_EQ(expected, seq.reverseComplementData());
}

TEST(Sequence, fromStream) {
    stringstream ss("ACGTACGTACGT");
    string expected = "ACGTA";
    Sequence seq(ss, 5);
    ASSERT_EQ(expected, seq.data());
}

TEST(Sequence, null) {
    Sequence star("*");
    ASSERT_TRUE(star.null());

    Sequence zero("0");
    ASSERT_TRUE(zero.null());

    Sequence dash("-");
    ASSERT_TRUE(dash.null());
}
