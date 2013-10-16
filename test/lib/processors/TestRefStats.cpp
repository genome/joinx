#include "processors/RefStats.hpp"

#include <boost/assign/list_of.hpp>

#include <gtest/gtest.h>

#include <vector>
#include <string>

using boost::assign::list_of;

TEST(TestRefStats, match) {
    std::vector<std::string> v = list_of("A")("C")("G")("T")("CG")("AT");
    RefStats stats(v);
    std::string ref("A" "CGCG" "T" "AT" "TA" "CG");
    stats.match(ref);
    EXPECT_EQ(2u, stats.count("A"));
    EXPECT_EQ(0u, stats.count("C"));
    EXPECT_EQ(2u, stats.count("T"));
    EXPECT_EQ(0u, stats.count("G"));
    EXPECT_EQ(1u, stats.count("AT"));
    EXPECT_EQ(3u, stats.count("CG"));
}

TEST(TestRefStats, nested) {
    std::vector<std::string> v = list_of("ACGT")("AC")("CG")("GT");
    std::string ref("ACGT" "ACN" "CGN" "GTN");

    RefStats stats(v);
    stats.match(ref);

    EXPECT_EQ(1u, stats.count("ACGT"));
    EXPECT_EQ(1u, stats.count("AC"));
    EXPECT_EQ(1u, stats.count("CG"));
    EXPECT_EQ(1u, stats.count("GT"));
}
