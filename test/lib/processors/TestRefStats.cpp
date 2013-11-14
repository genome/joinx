#include "processors/RefStats.hpp"

#include "fileformats/Fasta.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>
#include <string>

using boost::assign::list_of;
using boost::format;

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

TEST(TestTokenSpec, invalidToken) {
    std::vector<std::string> invalid = list_of
            ("A+C")
            ("A|C")
            ("A.*C")
            ("A.+G")
            ("A?G")
            ;

    for (auto i = invalid.begin(); i != invalid.end(); ++i) {
        std::vector<std::string> x(1, *i);
        EXPECT_THROW((TokenSpec(x)), InvalidTokenError);
    }
}

TEST(TestTokenSpec, duplicateTokens) {
    std::vector<std::string> ok = list_of("A")("AC")("G/T");
    EXPECT_NO_THROW((TokenSpec(ok)));

    {
        std::vector<std::string> dup = list_of("A")("AC")("A/T");
        EXPECT_THROW((TokenSpec(dup)), DuplicateTokenError);
    }
    {
        std::vector<std::string> dup = list_of("C")("AC")("C/T");
        EXPECT_THROW((TokenSpec(dup)), DuplicateTokenError);
    }
    {
        std::vector<std::string> dup = list_of("A")("AC")("A");
        EXPECT_THROW((TokenSpec(dup)), DuplicateTokenError);
    }
    {
        std::vector<std::string> dup = list_of("A")("AC")("AC/T");
        EXPECT_THROW((TokenSpec(dup)), DuplicateTokenError);
    }
}

TEST(TestTokenSpec, tokenMap) {
    std::vector<std::string> tokens = list_of("a/t")("c/g")("cg");
    std::vector<std::string> expectedTokens = list_of("CG")("A")("T")("C")("G");

    TokenSpec spec(tokens);
    EXPECT_EQ(expectedTokens, spec.tokens());
    EXPECT_EQ(tokens, spec.origTokens());

    EXPECT_EQ("a/t", spec.tokenFor("A"));
    EXPECT_EQ("a/t", spec.tokenFor("T"));

    EXPECT_EQ("c/g", spec.tokenFor("C"));
    EXPECT_EQ("c/g", spec.tokenFor("G"));

    EXPECT_EQ("cg", spec.tokenFor("CG"));

    EXPECT_THROW(spec.tokenFor("x"), std::runtime_error);
}
