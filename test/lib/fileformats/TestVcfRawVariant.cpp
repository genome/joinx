#include "fileformats/vcf/RawVariant.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "io/InputStream.hpp"
#include "common/VariantType.hpp"

#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Vcf;
using namespace std;
using boost::assign::list_of;

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

    RawVariant::Vector raw = RawVariant::processEntry(e);
    ASSERT_EQ(nExpected, raw.size());
    for (size_t i = 0; i < nExpected; ++i) {
        EXPECT_EQ(expected[i].pos, raw[i].pos) << " at index " << i << " in\n" << e;
        EXPECT_EQ(expected[i].ref, raw[i].ref) << " at index " << i << " in\n" << e;
        EXPECT_EQ(expected[i].alt, raw[i].alt) << " at index " << i << " in\n" << e;
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

    RawVariant::Vector raw = RawVariant::processEntry(e);
    ASSERT_EQ(nExpected, raw.size());
    for (size_t i = 0; i < nExpected; ++i) {
        EXPECT_EQ(expected[i].pos, raw[i].pos) << " at index " << i << " in\n" << e;
        EXPECT_EQ(expected[i].ref, raw[i].ref) << " at index " << i << " in\n" << e;
        EXPECT_EQ(expected[i].alt, raw[i].alt) << " at index " << i << " in\n" << e;
    }
}

TEST_F(TestVcfRawVariant, lastPos) {
    RawVariant snv(10, "A", "C");
    EXPECT_EQ(10, snv.lastRefPos());
    EXPECT_EQ(10, snv.lastAltPos());

    RawVariant ins(10, "A", "AC");
    EXPECT_EQ(10, ins.lastRefPos());
    EXPECT_EQ(11, ins.lastAltPos());

    RawVariant del(10, "AC", "A");
    EXPECT_EQ(11, del.lastRefPos());
    EXPECT_EQ(10, del.lastAltPos());
}

TEST_F(TestVcfRawVariant, split_merge_IndelWithSubstitution_insertion) {
    RawVariant var(10, "CAC", "TACGT");
    pair<RawVariant, RawVariant> pear = var.splitIndelWithSubstitution();

    EXPECT_EQ(RawVariant(10, "C", "T"), pear.first);
    EXPECT_EQ(RawVariant(13, "", "GT"), pear.second);

    RawVariant merged = var.mergeIndelWithSubstitution(pear);
    EXPECT_EQ(var, merged);
}

TEST_F(TestVcfRawVariant, split_merge_IndelWithSubstitution_deletion) {
    RawVariant var(10, "CACGT", "TAC");
    pair<RawVariant, RawVariant> pear = var.splitIndelWithSubstitution();
    EXPECT_EQ(RawVariant(10, "C", "T"), pear.first);
    EXPECT_EQ(RawVariant(13, "GT", ""), pear.second);

    RawVariant merged = var.mergeIndelWithSubstitution(pear);
    EXPECT_EQ(var, merged);
}

TEST_F(TestVcfRawVariant, split_merge_IndelWithSubstitution_noIndel) {
    RawVariant var(10, "TACG", "GACT");
    EXPECT_EQ(10, var.pos);
    EXPECT_EQ("TACG", var.ref);
    EXPECT_EQ("GACT", var.alt);
    pair<RawVariant, RawVariant> pear = var.splitIndelWithSubstitution();
    EXPECT_EQ(var, pear.first);
    EXPECT_EQ(RawVariant::None, pear.second);

    RawVariant merged = var.mergeIndelWithSubstitution(pear);
    EXPECT_EQ(var, merged);

    pear = RawVariant::None.splitIndelWithSubstitution();
    EXPECT_EQ(RawVariant::None, pear.first);
    EXPECT_EQ(RawVariant::None, pear.second);

    merged = var.mergeIndelWithSubstitution(pear);
    EXPECT_EQ(RawVariant::None, merged);
}

TEST_F(TestVcfRawVariant, combineRefAllelesOverlap) {
    std::vector<RawVariant> vars = list_of
        (RawVariant(10, "AC", ""))
        (RawVariant(11, "CG", ""))
        (RawVariant(12, "GT", ""))
        ;

    std::string result = Vcf::RawVariant::combineRefAlleles(vars);
    EXPECT_EQ("ACGT", result);
}

TEST_F(TestVcfRawVariant, combineRefAllelesAdjacent) {
    std::vector<RawVariant> vars = list_of
        (RawVariant(10, "AC", ""))
        (RawVariant(12, "CG", ""))
        (RawVariant(14, "GT", ""))
        ;

    std::string result = Vcf::RawVariant::combineRefAlleles(vars);
    EXPECT_EQ("ACCGGT", result);
}

TEST_F(TestVcfRawVariant, combineRefAllelesGap) {
    std::vector<RawVariant> vars = list_of
        (RawVariant(10, "AC", ""))
        (RawVariant(13, "CG", ""))
        (RawVariant(16, "GT", ""))
        ;

    std::string result = Vcf::RawVariant::combineRefAlleles(vars);
    EXPECT_EQ("AC.CG.GT", result);
}

TEST_F(TestVcfRawVariant, isSimpleIndel) {
    RawVariant rv(10, "ACC", "");
    for (int i = 0; i < 2; ++i) {
        EXPECT_FALSE(isSimpleIndel(rv, 0));
        EXPECT_FALSE(isSimpleIndel(rv, 1));
        EXPECT_FALSE(isSimpleIndel(rv, 2));
        EXPECT_TRUE(isSimpleIndel(rv, 3));
        EXPECT_TRUE(isSimpleIndel(rv, 4));
        rv.ref.swap(rv.alt);
    }
}

TEST_F(TestVcfRawVariant, allBasesMatch) {
    RawVariant mix(10, "CCA", "");
    RawVariant match(10, "AAA", "");

    for (int i = 0; i < 2; ++i) {
        EXPECT_FALSE(allBasesMatch('A', mix));
        EXPECT_FALSE(allBasesMatch('C', mix));
        EXPECT_FALSE(allBasesMatch('G', mix));
        EXPECT_FALSE(allBasesMatch('T', mix));

        EXPECT_TRUE(allBasesMatch('A', match));
        EXPECT_FALSE(allBasesMatch('C', match));
        EXPECT_FALSE(allBasesMatch('G', match));
        EXPECT_FALSE(allBasesMatch('T', match));

        mix.ref.swap(mix.alt);
        match.ref.swap(match.alt);
    }
}
