#include "processors/RefStats.hpp"

#include "fileformats/Fasta.hpp"

#include <boost/assign/list_of.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>
#include <string>

using boost::assign::list_of;

namespace {
    struct Reference {
        Reference(std::string const& bases)
            : size(bases.size())
            , bases(bases)
            , data(">1\n" + bases)
            , fasta("test", data.data(), data.size())
        {
        }

        size_t size;
        std::string bases;
        std::string data;
        Fasta fasta;
    };
}

TEST(TestRefStats, match) {
    std::vector<std::string> v = list_of("A")("C")("G")("T")("CG")("AT");
    Reference ref("A" "CGCG" "T" "AT" "T" "A" "CG");

    RefStats refStats(v, ref.fasta);
    Region region(0, ref.size);

    auto stats = refStats.match("1", region);

    EXPECT_EQ(2u, stats.count("A"));
    EXPECT_EQ(0u, stats.count("C"));
    EXPECT_EQ(2u, stats.count("T"));
    EXPECT_EQ(0u, stats.count("G"));
    EXPECT_EQ(2u, stats.count("AT"));
    EXPECT_EQ(6u, stats.count("CG"));

    EXPECT_EQ(ref.bases, stats.referenceString);
}

TEST(TestRefStats, nested) {
    std::vector<std::string> v = list_of("ACGT")("AC")("CG")("GT");
    Reference ref("ACGT" "ACN" "CGN" "GTN");

    RefStats refStats(v, ref.fasta);
    Region region(0, ref.size);

    auto stats = refStats.match("1", region);

    EXPECT_EQ(4u, stats.count("ACGT"));
    EXPECT_EQ(2u, stats.count("AC"));
    EXPECT_EQ(2u, stats.count("CG"));
    EXPECT_EQ(2u, stats.count("GT"));

    EXPECT_EQ(ref.bases, stats.referenceString);
}

TEST(TestRefStats, edges) {
    std::vector<std::string> v = list_of("TCG")("CG")("A")("C")("G")("T");
    Reference ref(
            "aat"
            "tCG"
            "CGG"
            "ACG"
            "AAT"
            "cgg");

    RefStats refStats(v, ref.fasta);

    auto stats = refStats.match("1", Region(5, 16));

    EXPECT_EQ(3u, stats.count("TCG"));
    EXPECT_EQ(4u, stats.count("CG"));
    EXPECT_EQ(3u, stats.count("A"));
    EXPECT_EQ(0u, stats.count("C"));
    EXPECT_EQ(1u, stats.count("G"));
    EXPECT_EQ(0u, stats.count("T"));

    EXPECT_EQ(ref.bases.substr(5, 11), stats.referenceString);
}
