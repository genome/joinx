#include "common/Sequence.hpp"

#include <sstream>
#include <string>
#include <gtest/gtest.h>

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

TEST(Sequence, commonPrefixReverseIterators) {
    string a("GCC");
    string b("GA");
    ASSERT_EQ(0u, Sequence::commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));

    a="GCC";
    b="GAC";
    ASSERT_EQ(1u, Sequence::commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));

    a="GCC";
    b="GCC";
    ASSERT_EQ(3u, Sequence::commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));
}
