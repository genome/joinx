#include "processors/VariantContig.hpp"

#include "common/UnknownSequenceError.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/vcf/RawVariant.hpp"

#include <string>
#include <gtest/gtest.h>

using namespace Vcf;
using namespace std;

class TestVcfVariantContig : public ::testing::Test {
protected:
    TestVcfVariantContig() 
        :_ref(">1\nAATGGCTAGGCTCC\n>2\nAAGGTTCCGACCTTGGAA\n>3\nAAGGTTCCGAGAGACCTTGGAA")
        ,_fa("test_fa", _ref.data(), _ref.size())
    {
    }

    void SetUp() {
        
    }
    
    string _ref;
    Fasta _fa;
};

TEST_F(TestVcfVariantContig, construct) {
    RawVariant raw(7, "T", "");
    VariantContig contig(raw, _fa, 3, "1");

    ASSERT_EQ("GGCAGG", contig.sequence());
    ASSERT_EQ(4, contig.start());
    ASSERT_EQ(10, contig.stop());
    ASSERT_EQ("3M1D3M", contig.cigar());
}

TEST_F(TestVcfVariantContig, frontEdge) {
    RawVariant raw(2, "A", "C");
    VariantContig contig(raw, _fa, 3, "1");

    ASSERT_EQ("ACTGG", contig.sequence());
    ASSERT_EQ(1, contig.start());
    ASSERT_EQ(5, contig.stop());
    ASSERT_EQ("5M", contig.cigar());
}

TEST_F(TestVcfVariantContig, backEdge) {
    RawVariant raw(13, "C", "G");
    VariantContig contig(raw, _fa, 3, "1");

    ASSERT_EQ("GCTGC", contig.sequence());
    ASSERT_EQ(10, contig.start());
    ASSERT_EQ(14, contig.stop());
    ASSERT_EQ("5M", contig.cigar());
}

TEST_F(TestVcfVariantContig, firstBase) {
    RawVariant raw(1, "A", "T");
    VariantContig contig(raw, _fa, 3, "1");

    ASSERT_EQ("TATG", contig.sequence());
    ASSERT_EQ(1, contig.start());
    ASSERT_EQ(4, contig.stop());
    ASSERT_EQ("4M", contig.cigar());
}

TEST_F(TestVcfVariantContig, lastBase) {
    RawVariant raw(14, "C", "G");
    VariantContig contig(raw, _fa, 3, "1");

    ASSERT_EQ("CTCG", contig.sequence());
    ASSERT_EQ(11, contig.start());
    ASSERT_EQ(14, contig.stop());
    ASSERT_EQ("4M", contig.cigar());
}

TEST_F(TestVcfVariantContig, paddingAtFirstBase) {
    RawVariant raw(3, "T", "C");
    VariantContig contig(raw, _fa, 3, "1");

    ASSERT_EQ("AACGGC", contig.sequence());
    ASSERT_EQ(1, contig.start());
    ASSERT_EQ(6, contig.stop());
    ASSERT_EQ("6M", contig.cigar());
}

TEST_F(TestVcfVariantContig, unknownSequence) {
    RawVariant raw(13, "C", "G");
    ASSERT_THROW(VariantContig(raw, _fa, 3, "x"),
        UnknownSequenceError);
}

TEST_F(TestVcfVariantContig, substPlusInsertion) {
    RawVariant raw(9, "GA", "TGTGTGTG");
    VariantContig contig(raw, _fa, 8, "2");

    ASSERT_EQ("AAGGTTCCTGTGTGTGCCTTGGAA", contig.sequence());
    ASSERT_EQ(1, contig.start());
    ASSERT_EQ(18, contig.stop());
    ASSERT_EQ("10M6I8M", contig.cigar());
}

TEST_F(TestVcfVariantContig, substPlusDeletion) {
    RawVariant raw(9, "GAGAGA", "TG");
    VariantContig contig(raw, _fa, 8, "3");

    ASSERT_EQ("AAGGTTCCTGCCTTGGAA", contig.sequence());
    ASSERT_EQ(1, contig.start());
    ASSERT_EQ(22, contig.stop());
    ASSERT_EQ("10M4D8M", contig.cigar());
}
