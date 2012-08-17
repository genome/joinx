#include "fileformats/vcf/RawVariant.hpp"
#include "processors/VariantContig.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/InputStream.hpp"
#include "common/VariantType.hpp"
#include "fileformats/Fasta.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace Vcf;
using namespace std::placeholders;
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

