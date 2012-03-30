#include "fileformats/WiggleReader.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

namespace {
    string WIG =
        "track type=wiggle_0 name=SomaticCoverage viewLimits=0:1\n"
        "fixedStep chrom=chr1 start=99 step=1\n"
        "2\n"
        "0\n"
        "0\n"
        "0\n"
        "1\n"
        "0\n"
        "0\n"
        "0\n"
        "fixedStep chrom=chr2 start=100 step=1\n"
        "1\n"
        "2\n"
        "2\n"
        "3\n"
        "fixedStep chrom=chr3 start=100 step=10\n"
        "1\n"
        "1\n"
        "1\n"
        "fixedStep chrom=chr4 start=100 step=10 span=2\n"
        "1\n"
        "1\n"
        "1\n"
    ;
}


TEST(WiggleReader, noStrip) {
    WiggleReader wr("TEST", WIG.data(), WIG.size(), false);
    Bed entry;
    // chr 1
    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("chr1\t98\t99\t2", entry.toString());
}

TEST(WiggleReader, parse) {
    WiggleReader wr("TEST", WIG.data(), WIG.size(), true);
    Bed entry;
    // chr 1
    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("1\t98\t99\t2", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("1\t99\t102\t0", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("1\t102\t103\t1", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("1\t103\t106\t0", entry.toString());

    // chr 2
    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("2\t99\t100\t1", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("2\t100\t102\t2", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("2\t102\t103\t3", entry.toString());

    // chr 3
    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("3\t99\t100\t1", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("3\t109\t110\t1", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("3\t119\t120\t1", entry.toString());

    // chr 4
    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("4\t99\t101\t1", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("4\t109\t111\t1", entry.toString());

    ASSERT_TRUE(wr.next(entry));
    ASSERT_EQ("4\t119\t121\t1", entry.toString());

    ASSERT_TRUE(wr.eof());
    ASSERT_FALSE(wr.next(entry));
}
