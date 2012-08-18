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
        :_ref(">1\nAATGGCTAGGCTCC")
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
    VariantContig variant(raw, _fa, 3, "1");

    ASSERT_EQ("GGCAGG", variant.sequence());
    ASSERT_EQ(4, variant.start());
    ASSERT_EQ(10, variant.stop());
    ASSERT_EQ("3M1D3M", variant.cigar());
}

TEST_F(TestVcfVariantContig, frontEdge) {
    RawVariant raw(2, "A", "C");
    VariantContig variant(raw, _fa, 3, "1");

    ASSERT_EQ("ACTGG", variant.sequence());
    ASSERT_EQ(1, variant.start());
    ASSERT_EQ(5, variant.stop());
    ASSERT_EQ("5M", variant.cigar());
}

TEST_F(TestVcfVariantContig, backEdge) {
    RawVariant raw(13, "C", "G");
    VariantContig variant(raw, _fa, 3, "1");

    ASSERT_EQ("GCTGC", variant.sequence());
    ASSERT_EQ(10, variant.start());
    ASSERT_EQ(14, variant.stop());
    ASSERT_EQ("5M", variant.cigar());
}

TEST_F(TestVcfVariantContig, unknownSequence) {
    RawVariant raw(13, "C", "G");
    ASSERT_THROW(VariantContig(raw, _fa, 3, "2"),
        UnknownSequenceError);
}
