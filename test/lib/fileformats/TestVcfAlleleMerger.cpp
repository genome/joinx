#include "fileformats/vcf/AlleleMerger.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/InputStream.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Vcf;
using namespace std;

/* from Dave:

1    199885139    .    T    TCG
1    199885140    .    C    CGC
1    199885141    .    G    GCG

should be recognized as all identical and merged into a single entry.
See alignments below for the explanation

If the reference sequence is CTCGG then you could get the following 3
alignments, all equivalent:

REF            CTCG--G        1    199885140    199885141    0    CG
Indel2         CTCGCGG

REF            CTC--GG        1    199885139    199885140    0    GC
Indel2         CTCGCGG

REF            CT--CGG        1    199885138    199885139    0    CG
Indel2         CTCGCGG

*/

class TestVcfAlleleMerger : public ::testing::Test {
protected:
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

TEST_F(TestVcfAlleleMerger, insertion) {
    // reference: CTCGG at 1,38

    vector<Entry> ents;
    // the next 3 are equivalent
    ents.push_back(makeEntry("1", 39, "T", "TCG"));
    ents.push_back(makeEntry("1", 40, "C", "CGC"));
    ents.push_back(makeEntry("1", 41, "G", "GCG"));
    // this one is different
    ents.push_back(makeEntry("1", 41, "G", "GTG"));

    AlleleMerger am(ents);

    // yes, something was merged.
    ASSERT_TRUE(am.merged());

    EXPECT_EQ("TCG", am.ref());
    // make sure the total # of (unique) alts is now 2
    EXPECT_EQ(2u, am.mergedAlt().size());
    EXPECT_EQ("TCGCG", am.mergedAlt()[0]);
    EXPECT_EQ("TCGTG", am.mergedAlt()[1]); // the extra guy we threw in there

    // check genotype mapping for each sample
    // this is an array of arrays (one per sample) that maps indices into
    // the old alt array into the new merged one
    ASSERT_EQ(4u, am.newAltIndices().size());

    // the first 3 entries should all be merged so their old index of
    // 0 should point to 0 again in the merged alts
    for (size_t i = 0; i < 3; ++i)
        EXPECT_EQ(0u, am.newAltIndices()[i][0]);

    // the different guy's old index of 0 should now point to 1 in
    // the merged array
    EXPECT_EQ(1u, am.newAltIndices()[3][0]);
}

TEST_F(TestVcfAlleleMerger, deletion) {
    vector<Entry> ents;
    // the next 3 are equivalent
    ents.push_back(makeEntry("1", 39, "TG", "T"));
    ents.push_back(makeEntry("1", 40, "GG", "G"));
    ents.push_back(makeEntry("1", 41, "GG", "G"));

    AlleleMerger am(ents);

    // yes, something was merged.
    ASSERT_TRUE(am.merged());

    EXPECT_EQ("TGGG", am.ref());
    EXPECT_EQ(1u, am.mergedAlt().size());
    EXPECT_EQ("TGG", am.mergedAlt()[0]);

    ASSERT_EQ(3u, am.newAltIndices().size());

    for (size_t i = 0; i < 3; ++i)
        EXPECT_EQ(0u, am.newAltIndices()[i][0]);
}

TEST_F(TestVcfAlleleMerger, snv) {
    vector<Entry> ents;
    // the next 3 are equivalent
    ents.push_back(makeEntry("1", 39, "TGA", "TGC"));
    ents.push_back(makeEntry("1", 40, "GA", "GC"));
    ents.push_back(makeEntry("1", 41, "A", "C"));

    AlleleMerger am(ents);
    ASSERT_TRUE(am.merged());
    EXPECT_EQ("TGA", am.ref());
    EXPECT_EQ(1u, am.mergedAlt().size());
    EXPECT_EQ("TGC", am.mergedAlt()[0]);

    ASSERT_EQ(3u, am.newAltIndices().size());

    for (size_t i = 0; i < 3; ++i)
        EXPECT_EQ(0u, am.newAltIndices()[i][0]);
}

TEST_F(TestVcfAlleleMerger, identical) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 39, "T", "TG"));
    ents.push_back(makeEntry("1", 39, "T", "TG"));

    AlleleMerger am(ents);

    ASSERT_TRUE(am.merged());
    EXPECT_EQ("T", am.ref());
    EXPECT_EQ(1u, am.mergedAlt().size());
    EXPECT_EQ("TG", am.mergedAlt()[0]);

    ASSERT_EQ(2u, am.newAltIndices().size());
    EXPECT_EQ(0u, am.newAltIndices()[0][0]);
    EXPECT_EQ(0u, am.newAltIndices()[1][0]);
}

TEST_F(TestVcfAlleleMerger, overlapButNoMerge) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 39, "T", "TG"));
    ents.push_back(makeEntry("1", 39, "T", "TC"));

    AlleleMerger am(ents);
    ASSERT_TRUE(am.merged());
    EXPECT_EQ("T", am.ref());
    ASSERT_EQ(2u, am.mergedAlt().size());
    EXPECT_EQ("TG", am.mergedAlt()[0]);
    EXPECT_EQ("TC", am.mergedAlt()[1]);
    EXPECT_EQ(0u, am.newAltIndices()[0][0]);
    EXPECT_EQ(1u, am.newAltIndices()[1][0]);
}

TEST_F(TestVcfAlleleMerger, mismatchChrom) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 39, "T", "TC"));
    ents.push_back(makeEntry("2", 39, "T", "TC"));

    AlleleMerger am(ents);
    ASSERT_FALSE(am.merged());
}

TEST_F(TestVcfAlleleMerger, buildRefAdjacent) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 10, "ACG", "A"));
    ents.push_back(makeEntry("1", 13, "TGA", "T"));
    ents.push_back(makeEntry("1", 16, "CGA", "C"));
    string ref = AlleleMerger::buildRef(&*ents.begin(), &*ents.end());
    EXPECT_EQ("ACGTGACGA", ref);
}

TEST_F(TestVcfAlleleMerger, buildRefOverlap) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 10, "ACG", "A"));
    ents.push_back(makeEntry("1", 12,   "GTA", "T"));
    ents.push_back(makeEntry("1", 13,    "TAG", "C"));
    string ref = AlleleMerger::buildRef(&*ents.begin(), &*ents.end());
    EXPECT_EQ("ACGTAG", ref);
}

TEST_F(TestVcfAlleleMerger, buildRefExactMatch) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 10, "ACGT", "CGT"));
    ents.push_back(makeEntry("1", 10, "ACGT", "GT"));
    string ref = AlleleMerger::buildRef(&*ents.begin(), &*ents.end());
    EXPECT_EQ("ACGT", ref);
}

TEST_F(TestVcfAlleleMerger, buildRefDisjoint) {
    // when there are gaps in the reference, an empty string should be returned
    // since we cannot infer the sequence.
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 10, "ACG", "A"));
    ents.push_back(makeEntry("1", 14, "TGA", "T"));
    ents.push_back(makeEntry("1", 17, "CGA", "C"));
    string ref = AlleleMerger::buildRef(&*ents.begin(), &*ents.end());
    EXPECT_EQ("", ref);
}

TEST_F(TestVcfAlleleMerger, nullAlt) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 10, "A", "T"));
    ents.push_back(makeEntry("1", 10, "A", "G"));
    ents.push_back(makeEntry("1", 10, "A", "."));

    AlleleMerger am(ents);
    ASSERT_TRUE(am.merged());
    ASSERT_EQ(2u, am.mergedAlt().size());
    ASSERT_EQ(3u, am.newAltIndices().size());
    EXPECT_EQ(0u, am.newAltIndices()[0][0]);
    EXPECT_EQ(1u, am.newAltIndices()[1][0]);
    ASSERT_TRUE(am.newAltIndices()[2].empty());
}

TEST_F(TestVcfAlleleMerger, adjacentDeletion) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 9,  "CAGGAGTCCAGCGCAG", "C"));
    ents.push_back(makeEntry("1", 10,  "AGG", "A"));
    ents.push_back(makeEntry("1", 13,     "AGTCCAGCGCAG", "A"));
    AlleleMerger am(ents);
    ASSERT_TRUE(am.merged());
    ASSERT_EQ(3u, am.mergedAlt().size());
    ASSERT_TRUE(am.newAltIndices().size());

    string ref = AlleleMerger::buildRef(&*ents.begin(), &*ents.end());
    EXPECT_EQ("CAGGAGTCCAGCGCAG", ref);
}

TEST_F(TestVcfAlleleMerger, deletionPaddingAfter) {
    vector<Entry> ents;
    ents.push_back(makeEntry("1", 10, "ACGT", "CGT"));
    ents.push_back(makeEntry("1", 10, "ACGT", "CG"));

    AlleleMerger am(ents);
    ASSERT_TRUE(am.merged());
    EXPECT_EQ("ACGT", am.ref());
    ASSERT_EQ(2u, am.mergedAlt().size());
    EXPECT_EQ("CGT", am.mergedAlt()[0]);
    EXPECT_EQ("CG", am.mergedAlt()[1]);
}
