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

TEST(Fasta, seq) {
    stringstream dat(goodData);
    Fasta fa("test", goodData.c_str(), goodData.size());

    ASSERT_EQ("CAGAGGTCCT", fa.sequence("1", 56, 10));
    ASSERT_EQ("CAAA", fa.sequence("1", 241, 4));
    ASSERT_EQ("CTA", fa.sequence("1", 279, 3));

    ASSERT_EQ("TTGT", fa.sequence("2",   1, 4));
    ASSERT_EQ("TAGA", fa.sequence("2",  61, 4));
    ASSERT_EQ("TGTT", fa.sequence("2", 121, 4));
    ASSERT_EQ("AATC", fa.sequence("2", 181, 4));
    ASSERT_EQ("TATA", fa.sequence("2", 241, 4));

    ASSERT_EQ("ACAA", fa.sequence("2",  57, 4));
    ASSERT_EQ("TGGA", fa.sequence("2", 117, 4));
    ASSERT_EQ("CATA", fa.sequence("2", 177, 4));
    ASSERT_EQ("ACGT", fa.sequence("2", 237, 4));
    ASSERT_EQ("GTGT", fa.sequence("2", 294, 4));

    ASSERT_THROW(fa.sequence("3", 0, 1), runtime_error);
    ASSERT_THROW(fa.sequence("3", 5, 1), length_error);
    ASSERT_EQ("ACGT", fa.sequence("3", 1, 4));

    ASSERT_EQ("T", fa.sequence("3", 4, 1));
}

TEST(Fasta, posOverflow) {
    stringstream dat(goodData);
    Fasta fa("test", goodData.c_str(), goodData.size());
    ASSERT_THROW(fa.sequence("1", (uint64_t)-1, 10), length_error);
    
}
