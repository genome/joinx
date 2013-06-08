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

namespace {
    ostream& operator<<(ostream& s, RawVariant const& rv) {
        return s << rv.pos << " ref:" << rv.ref << ", alt:" << rv.alt;
    }
}

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
TEST_F(TestVcfRawVariant, singlealt) {
    Entry e = makeEntry("1", 10, "CCCC", 
        "C,"        // 1) 3bp deletion of CCC at position 11
        "CCCG,"     // 2) C/G snp at position 13
        "CGCC,"     // 3) C/G snp at position 11
        "CCTG,"     // 4) CG/TG dnp at position 13
        "CCCCG,"    // 5) 1bp insertion of G at position 14
        "CGCCC"     // 6) 1bp insertion of G at position 11
        );

    RawVariant expected[] = {
        RawVariant(11, "CCC", ""),
        RawVariant(13, "C", "G"),
        RawVariant(11, "C", "G"),
        RawVariant(12, "CC", "TG"),
        RawVariant(14, "", "G"),
        RawVariant(11, "", "G")
    };
    size_t nExpected = sizeof(expected) / sizeof(expected[0]);

    vector<RawVariant> raw = RawVariant::processEntry(e);
    ASSERT_EQ(nExpected, raw.size());
    for (size_t i = 0; i < nExpected; ++i) {
        ASSERT_EQ(expected[i].pos, raw[i].pos) << " at index " << i << " in\n" << e;
        ASSERT_EQ(expected[i].ref, raw[i].ref) << " at index " << i << " in\n" << e;
        ASSERT_EQ(expected[i].alt, raw[i].alt) << " at index " << i << " in\n" << e;
    }
}

// double alt cases
TEST_F(TestVcfRawVariant, doublealt) {
    Entry e = makeEntry("1", 10, "CTA", 
        "C,"
        "CA"
        );

    RawVariant expected[] = {
        RawVariant(11, "TA", ""),
        RawVariant(11, "T", "")
    };
    size_t nExpected = sizeof(expected) / sizeof(expected[0]);

    vector<RawVariant> raw = RawVariant::processEntry(e);
    ASSERT_EQ(nExpected, raw.size());
    for (size_t i = 0; i < nExpected; ++i) {
        ASSERT_EQ(expected[i].pos, raw[i].pos) << " at index " << i << " in\n" << e;
        ASSERT_EQ(expected[i].ref, raw[i].ref) << " at index " << i << " in\n" << e;
        ASSERT_EQ(expected[i].alt, raw[i].alt) << " at index " << i << " in\n" << e;
    }
}

TEST_F(TestVcfRawVariant, lastPos) {
    RawVariant snv(10, "A", "C");
    ASSERT_EQ(10, snv.lastRefPos());
    ASSERT_EQ(10, snv.lastAltPos());

    RawVariant ins(10, "A", "AC");
    ASSERT_EQ(10, ins.lastRefPos());
    ASSERT_EQ(11, ins.lastAltPos());

    RawVariant del(10, "AC", "A");
    ASSERT_EQ(11, del.lastRefPos());
    ASSERT_EQ(10, del.lastAltPos());
}

TEST_F(TestVcfRawVariant, split_merge_IndelWithSubstitution_insertion) {
    RawVariant var(10, "CAC", "TACGT");
    pair<RawVariant, RawVariant> pear = var.splitIndelWithSubstitution();

    ASSERT_EQ(RawVariant(10, "C", "T"), pear.first);
    ASSERT_EQ(RawVariant(13, "", "GT"), pear.second);

    RawVariant merged = var.mergeIndelWithSubstitution(pear);
    ASSERT_EQ(var, merged);
}

TEST_F(TestVcfRawVariant, split_merge_IndelWithSubstitution_deletion) {
    RawVariant var(10, "CACGT", "TAC");
    pair<RawVariant, RawVariant> pear = var.splitIndelWithSubstitution();
    ASSERT_EQ(RawVariant(10, "C", "T"), pear.first);
    ASSERT_EQ(RawVariant(13, "GT", ""), pear.second);

    RawVariant merged = var.mergeIndelWithSubstitution(pear);
    ASSERT_EQ(var, merged);
}

TEST_F(TestVcfRawVariant, split_merge_IndelWithSubstitution_noIndel) {
    RawVariant var(10, "TACG", "GACT");
    ASSERT_EQ(10, var.pos);
    ASSERT_EQ("TACG", var.ref);
    ASSERT_EQ("GACT", var.alt);
    pair<RawVariant, RawVariant> pear = var.splitIndelWithSubstitution();
    ASSERT_EQ(var, pear.first);
    ASSERT_EQ(RawVariant::None, pear.second);

    RawVariant merged = var.mergeIndelWithSubstitution(pear);
    ASSERT_EQ(var, merged);

    pear = RawVariant::None.splitIndelWithSubstitution();
    ASSERT_EQ(RawVariant::None, pear.first);
    ASSERT_EQ(RawVariant::None, pear.second);

    merged = var.mergeIndelWithSubstitution(pear);
    ASSERT_EQ(RawVariant::None, merged);
}
