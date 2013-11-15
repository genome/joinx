#include "fileformats/Fasta.hpp"

#include <gtest/gtest.h>
#include <string>
#include <stdexcept>

using namespace std;

namespace {
    string goodData(
        ">1 test\n"
        // 60 bases per line
        "TATTATGTTTCAAGTGATGTTTAATTTAAGACTTTTCAGTTTGACGTTATTGCAACAGAG\n"
        "GTCCTGACACAAGTTTGTCAGCTTCTCCAAGCTTCTGTCTGTTCATTGGTTTAATGGGCA\n"
        "TTGTAGAGATGGGGTTTTGCCATGTTGGCCAGGCTGGTCTCGAACTTCTGGGCTCATGGA\n"
        "ATCTGCCCACCTAGGCCTCCCAGAATGCTGGGATTACAGGCATGAGTCACTGTGCTCGGC\n"
        "CAAACTATCATTCTATTTTTTTTAAAAAATCATATTATCTA\n" //41 bases
        ">2 test2\n"
        "TTGTAAAAATATGAGACAGTTAATTTAAATTCATATCATTGATTATGTATATGGAGACAA\n"
        "TAGAAGCAGCTTTCTTTTAAAAGACATACGCTCAACATAAGCTTAACTGAACGACATGGA\n"
        "TGTTAATAAAACTTTTCTGGCTTTCCAAGCCGTGATTTTATTTAAAACTGAATGGACATA\n"
        "AATCCCCTCTCATTTGCCAAATATAATTTAATCATGCGATATGTTATGGTTATTAAACGT\n"
        "TATATATTAGAGGGGGATGATCTGGGCGTGAATCCTAGCCTTACTATTTAGGTGTGT\n" // 57
        ">3 shorty\n"
        "ACGT\n"
    );
}

TEST(TestFasta, seq) {
    Fasta fa("test", goodData.c_str(), goodData.size());

    EXPECT_EQ("CAGAGGTCCT", fa.sequence("1", 56, 10));
    EXPECT_EQ("CAAA", fa.sequence("1", 241, 4));
    EXPECT_EQ("CTA", fa.sequence("1", 279, 3));

    EXPECT_EQ("TTGT", fa.sequence("2",   1, 4));
    EXPECT_EQ("TAGA", fa.sequence("2",  61, 4));
    EXPECT_EQ("TGTT", fa.sequence("2", 121, 4));
    EXPECT_EQ("AATC", fa.sequence("2", 181, 4));
    EXPECT_EQ("TATA", fa.sequence("2", 241, 4));

    EXPECT_EQ("ACAA", fa.sequence("2",  57, 4));
    EXPECT_EQ("TGGA", fa.sequence("2", 117, 4));
    EXPECT_EQ("CATA", fa.sequence("2", 177, 4));
    EXPECT_EQ("ACGT", fa.sequence("2", 237, 4));
    EXPECT_EQ("GTGT", fa.sequence("2", 294, 4));

    EXPECT_THROW(fa.sequence("3", 0, 1), runtime_error);
    EXPECT_THROW(fa.sequence("3", 5, 1), length_error);
    EXPECT_EQ("ACGT", fa.sequence("3", 1, 4));

    EXPECT_EQ("T", fa.sequence("3", 4, 1));

    auto const& index = fa.index();
    auto const& seqOrder = index.sequenceOrder();
    ASSERT_EQ(3u, seqOrder.size());
    EXPECT_EQ("1", seqOrder[0]);
    EXPECT_EQ("2", seqOrder[1]);
    EXPECT_EQ("3", seqOrder[2]);

    ASSERT_TRUE(index.entry("1"));
    // line length includes the newline
    EXPECT_EQ(61u, index.entry("1")->lineLength);
    EXPECT_EQ(60u * 4u + 41u, index.entry("1")->len);

    ASSERT_TRUE(index.entry("2"));
    EXPECT_EQ(61u, index.entry("2")->lineLength);
    EXPECT_EQ(60u * 4u + 57u, index.entry("2")->len);

    ASSERT_TRUE(index.entry("3"));
    EXPECT_EQ(5u, index.entry("3")->lineLength);
    EXPECT_EQ(4u, index.entry("3")->len);

    EXPECT_EQ(0, index.entry("4"));
}

TEST(TestFasta, posOverflow) {
    stringstream dat(goodData);
    Fasta fa("test", goodData.c_str(), goodData.size());
    EXPECT_THROW(fa.sequence("1", (uint64_t)-1, 10), length_error);
}

TEST(TestFasta, seq2) {
    string dat(">1\nACGT");
    Fasta fa("test", dat.data(), dat.size());
    string seq = fa.sequence("1", 1, fa.seqlen("1"));
    EXPECT_EQ("ACGT", seq);
}
