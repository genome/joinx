#include "fileformats/vcf/RawVariant.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/InputStream.hpp"
#include "common/VariantType.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace Vcf;
using namespace std::placeholders;
using namespace std;

class TestVcfRawVariant : public ::testing::Test {
protected:
    TestVcfRawVariant() {
    }

    void SetUp() {
        stringstream hdrss(
            "##fileformat=VCFv4.1\n"
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\n"
            );

        InputStream in("test", hdrss);
        _header = Header::fromStream(in);
    }

    Entry makeEntry(string chrom, int64_t pos, string const& ref, string const& alt) {
        stringstream ss;
        ss << chrom << "\t" << pos << "\t.\t" << ref << "\t" << alt << "\t.\t.\t.";
        return Entry(&_header, ss.str());
    }

    Header _header;
};

// single alt cases
TEST_F(TestVcfRawVariant, construct) {
    Entry e = makeEntry("1", 10, "CCCC", 
        "C,"        // 1) 3bp deletion of CCC at position 11
        "CCCG,"     // 2) C/G snp at position 13
        "CGCC,"      // 3) C/G snp at position 11
        "CCTG,"     // 4) CG/TG dnp at position 13
        "CCCCG,"    // 5) 1bp insertion of G at position 14
        "CGCCC"    // 6) 1bp insertion of G at position 11
        );

    RawVariant expected[] = {
        RawVariant(11, "CCC", ""),
        RawVariant(13, "C", "G"),
        RawVariant(11, "C", "G"),
        RawVariant(12, "CC", "TG"),
        RawVariant(14, "", "G"),
        RawVariant(11, "", "G")
    };
    int nExpected = sizeof(expected) / sizeof(expected[0]);

    vector<RawVariant> raw = RawVariant::processEntry(e);
    ASSERT_EQ(nExpected, raw.size());
    for (int i = 0; i < nExpected; ++i) {
        ASSERT_EQ(expected[i].pos, raw[i].pos) << " at index " << i << " in\n" << e;
        ASSERT_EQ(expected[i].ref, raw[i].ref) << " at index " << i << " in\n" << e;
        ASSERT_EQ(expected[i].alt, raw[i].alt) << " at index " << i << " in\n" << e;
    }
}
