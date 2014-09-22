#include "fileformats/vcf/AltNormalizer.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/Fasta.hpp"
#include "io/InputStream.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace Vcf;
using namespace std;

class TestVcfAltNormalizer : public ::testing::Test {
protected:
    TestVcfAltNormalizer()
        : _refFa(">1\nTTTCGCGCGCGCG")
        , _ref("test", _refFa.data(), _refFa.size())
    {
    }

    void SetUp() {
        stringstream hdrss(
            "##fileformat=VCFv4.1\n"
            "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\tS2\n"
            );

        InputStream in("test", hdrss);
        _header = Header::fromStream(in);
    }

    Entry makeEntry(string chrom, int64_t pos, string const& ref, string const& alt, string const& sampleData = ".\t.") {
        stringstream ss;
        ss << chrom << "\t" << pos << "\t.\t" << ref << "\t" << alt << "\t.\t.\t.\tGT\t" << sampleData;
        return Entry(&_header, ss.str());
    }

    string _refFa;
    Fasta _ref;
    Header _header;
};

// single alt cases
TEST_F(TestVcfAltNormalizer, insertion) {
    cout << "   REF: " << _ref.sequence("1", 1, 13) << "\n";
    string ref = _ref.sequence("1", 11, 3);
    EXPECT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "GCGCG");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(3u, e.pos());
    EXPECT_EQ("T", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("TCG", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, insertionWithTrailingRepeatMatch) {
    string ref = _ref.sequence("1", 11, 3);
    EXPECT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "GAGCG");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(10u, e.pos());
    EXPECT_EQ("C", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("CGA", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, immovableInsertion) {
    string ref = _ref.sequence("1", 11, 3);
    EXPECT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "GAATT");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    // We no longer strip padding from things that don't move.
    EXPECT_EQ(11u, e.pos());
    EXPECT_EQ("GCG", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("GAATT", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, deletion) {
    string ref = _ref.sequence("1", 11, 3);
    EXPECT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "G");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(3u, e.pos());
    EXPECT_EQ("TCG", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("T", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, deletionWithSubstitution) {
    string ref = _ref.sequence("1", 9, 5);
    EXPECT_EQ("GCGCG", ref);
    Entry e = makeEntry("1", 9, ref, "GAG");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    // We no longer strip padding from things that don't move.
    EXPECT_EQ(9u, e.pos());
    EXPECT_EQ("GCGCG", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("GAG", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, immovableDeletion) {
    string ref = _ref.sequence("1", 9, 5);
    EXPECT_EQ("GCGCG", ref);
    Entry e = makeEntry("1", 9, ref, "GAT");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    // We no longer strip padding from things that don't move.
    EXPECT_EQ(9u, e.pos());
    EXPECT_EQ("GCGCG", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("GAT", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, testSubstitution) {
    string ref = _ref.sequence("1", 9, 1);
    EXPECT_EQ("G", ref);
    Entry e = makeEntry("1", 9, ref, "C");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(9u, e.pos());
    EXPECT_EQ("G", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("C", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, testSubstitutionWithPadding) {
    string ref = _ref.sequence("1", 9, 5);
    EXPECT_EQ("GCGCG", ref);
    Entry e = makeEntry("1", 9, ref, "GCGCA");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    // We no longer strip padding from things that aren't moved.
    EXPECT_EQ(9u, e.pos());
    EXPECT_EQ("GCGCG", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("GCGCA", e.alt()[0]);
}

// multi alt cases
TEST_F(TestVcfAltNormalizer, insertionAndDeletion) {
    string ref = _ref.sequence("1", 11, 3);
    EXPECT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "GCGCG,GC");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";
}

TEST_F(TestVcfAltNormalizer, messyInsertionAndDeletion) {
    string refStr(">1\nTTTTTTTTTTTTTCCTCGCTCCC");
    Fasta ref("test", refStr.data(), refStr.size());
    Entry e = makeEntry("1", 22, "CC",  "C,CCTCGCTCCC");
    AltNormalizer n(ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";
    EXPECT_EQ(14u, e.pos());
    EXPECT_EQ(2u, e.alt().size());
    EXPECT_EQ("CCTCGCTC", e.ref());
    EXPECT_EQ("CCTCGCT", e.alt()[0]);
    EXPECT_EQ("CCTCGCTCCCTCGCTC", e.alt()[1]);
}

TEST_F(TestVcfAltNormalizer, indelAtPos1) {
    string refStr(">1\nAGAGAGAAAGAAAG");
    Fasta ref("test", refStr.data(), refStr.size());
    Entry e = makeEntry("1", 2, "GAG",  "G");
    AltNormalizer n(ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";
    EXPECT_EQ(1u, e.pos());
    EXPECT_EQ("AGA", e.ref());
    EXPECT_EQ("A", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, superfluousPaddingOmitted) {
    string refStr(">1\nCTTTTT");
    Fasta ref("test", refStr.data(), refStr.size());
    Entry e = makeEntry("1", 1, "CTT", "CAT,CTTTT");

    AltNormalizer n(ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(2u, e.pos());
    EXPECT_EQ("T", e.ref());
    ASSERT_EQ(2u, e.alt().size());
    EXPECT_EQ("A", e.alt()[0]);
    EXPECT_EQ("TTT", e.alt()[1]);
}

TEST_F(TestVcfAltNormalizer, trailingPadding) {
    string refStr(">1\nTATTATG");
    Fasta ref("test", refStr.data(), refStr.size());
    Entry e = makeEntry("1", 4, "TATG", "G");

    AltNormalizer n(ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(1u, e.pos());
    EXPECT_EQ("TATT", e.ref());
    ASSERT_EQ(1u, e.alt().size());
    EXPECT_EQ("T", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, pre_and_post_padding_bug) {
    // The bug was that when a variant at position 2 needs padding, it would
    // get it at both ends. (It gets added to the front first, which makes
    // its position 1, then the "need padding" flag was still set later and
    // oh look we're at position 1, better put the padding at the end)

    string refStr(">1\nATATTATG");
    Fasta ref("test", refStr.data(), refStr.size());
    Entry e = makeEntry("1", 3, "ATTA", "A");

    AltNormalizer n(ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(1u, e.pos());
    EXPECT_EQ("ATAT", e.ref());
    ASSERT_EQ(1u, e.alt().size());
    EXPECT_EQ("A", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, alts_equivalent_to_ref_are_removed) {
    string ref = _ref.sequence("1", 4, 6);
    EXPECT_EQ("CGCGCG", ref);
    Entry e = makeEntry("1", 4, ref, "CGCGCG,CGCG", "0/1\t1/2");
    AltNormalizer n(_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(3u, e.pos());
    EXPECT_EQ("TCG", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("T", e.alt()[0]);
    EXPECT_EQ("0/0", e.sampleData().genotype(0).string());
    EXPECT_EQ("0/1", e.sampleData().genotype(1).string());
}

TEST_F(TestVcfAltNormalizer, duplicate_alts_are_collapsed) {
    std::string refStr(">1\nATATTATG");
    Fasta ref("test", refStr.data(), refStr.size());
    Entry e = makeEntry("1", 4, "TTATG", "TG,TG", "0/1\t1/2");
    AltNormalizer n(ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    EXPECT_EQ(1u, e.pos());
    EXPECT_EQ("ATAT", e.ref());
    EXPECT_EQ(1u, e.alt().size());
    EXPECT_EQ("A", e.alt()[0]);
    EXPECT_EQ("0/1", e.sampleData().genotype(0).string());
    EXPECT_EQ("1/1", e.sampleData().genotype(1).string());
}
