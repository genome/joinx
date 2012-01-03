#include "fileformats/vcf/Entry.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/SampleData.hpp"

#include <gtest/gtest.h>
#include <sstream>

using namespace Vcf;
using namespace std;

namespace {
    string headerText(
        "##fileformat=VCFv4.1\n"
        "##INFO=<ID=AF,Number=A,Type=Float,Description=\"Allele Frequency\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\n"
        );

    string vcfLine =
        "1\t2\trs6054257\tG\tA\t29\ts50\tAF=0.5\tGT\t0|0\n"
        ;
}

class TestVcfMove : public ::testing::Test {
protected:
    void SetUp() {
        stringstream hdrss(headerText);
        InputStream in("test", hdrss);
        _header = Header::fromStream(in);
    }
    Header _header;
};

// this is an efficiency test
// it makes sure that move semantics are working properly (data is stolen from
// rvalue references in the move constructor)
TEST_F(TestVcfMove, move) {
    Entry e(&_header, vcfLine);
    char const* addrChrom = e.chrom().data();
    char const* addrRef = e.ref().data();
    string const* addrAlt = e.alt().data();
    string const* addrFilter = &*e.failedFilters().begin();
    Entry::CustomValueMap::value_type const* addrInfo = &*e.info().begin(); 

    // nested sample data addresses
    SampleData const& sd = e.sampleData();
    CustomType const* const* addrFormat = sd.format().data();
    SampleData::MapType::value_type const* addrSampleValues = &*sd.begin();

    // primitive types are copied, not moved. we still check that they get
    // set properly though.
    uint64_t origPos = e.pos();
    int64_t origStart = e.start();
    int64_t origStop = e.stop();
    double origQual = e.qual();

    // do the move and check that the resulting output strings are the same
    // first
    stringstream orig;
    orig << e;
    Entry e2(std::move(e));
    stringstream moved;
    moved << e2;
    ASSERT_EQ(orig.str(), moved.str());

    // now check that the addresses of data members of e2 have been stolen
    // from the original e
    ASSERT_EQ(addrChrom, e2.chrom().data());
    ASSERT_EQ(origPos, e2.pos());
    ASSERT_EQ(addrRef, e2.ref().data());
    ASSERT_EQ(addrAlt, e2.alt().data());
    ASSERT_EQ(origQual, e2.qual());
    ASSERT_EQ(addrFilter, &*e2.failedFilters().begin());
    ASSERT_EQ(addrInfo, &*e2.info().begin());
    ASSERT_EQ(addrFormat, e2.sampleData().format().data());
    ASSERT_EQ(addrSampleValues, &*e2.sampleData().begin());
    ASSERT_EQ(origStart, e2.start());
    ASSERT_EQ(origStop, e2.stop());
}
