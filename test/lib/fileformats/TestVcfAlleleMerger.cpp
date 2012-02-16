#include "fileformats/vcf/AlleleMerger.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/InputStream.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace Vcf;
using namespace std::placeholders;
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

namespace {
    // Mock reference sequence that acts like a fasta reader
    // initialize with the sequence to return as well as its
    // position (seqid, pos).
    class MockRef {
    public:
        MockRef(string const& ref, string const& seqid, int64_t pos)
            : _ref(ref)
            , _seqid(seqid)
            , _pos(pos)
        {
        }

        string sequence(string const& seqid, int32_t beg, int32_t end) {
            if (_seqid != seqid)
                throw runtime_error("wrong seqid");
            if (beg < _pos || end > _pos+int32_t(_ref.size()-1))
                throw runtime_error("seq out fo range");

            return _ref.substr(beg-_pos, end-beg+1);
        }

    protected:
        string _ref;
        string _seqid;
        int64_t _pos;
    };
}

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

TEST_F(TestVcfAlleleMerger, mockWorks) {
    // mock reference sequence with CTCGG starting at 1:38
    MockRef ref("CTCGG", "1", 38);

    ASSERT_EQ("C", ref.sequence("1", 38, 38));
    ASSERT_EQ("CT", ref.sequence("1", 38, 39));
    ASSERT_EQ("GG", ref.sequence("1", 41, 42));
}

TEST_F(TestVcfAlleleMerger, insertion) {
    // mock reference sequence with CTCGG starting at 1:38
    MockRef ref("CTCGG", "1", 38);

    vector<Entry> entries;
    // the next 3 are equivalent
    entries.push_back(makeEntry("1", 39, "T", "TCG"));
    entries.push_back(makeEntry("1", 40, "C", "CGC"));
    entries.push_back(makeEntry("1", 41, "G", "GCG"));
    // this one is different
    entries.push_back(makeEntry("1", 41, "G", "GTG"));

    AlleleMerger<MockRef> am(ref, entries);

    // yes, something was merged.
    ASSERT_TRUE(am.merged());

    // make sure the total # of (unique) alts is now 2
    ASSERT_EQ(2, am.mergedAlt().size());
    ASSERT_EQ("TCGCG", am.mergedAlt()[0]);
    ASSERT_EQ("TCGTG", am.mergedAlt()[1]); // the extra guy we threw in there

    // check genotype mapping for each sample
    // this is an array of arrays (one per sample) that maps indices into
    // the old alt array into the new merged one
    ASSERT_EQ(4, am.newGt().size());

    // the first 3 entries should all be merged so their old index of
    // 0 should point to 0 again in the merged alts
    for (size_t i = 0; i < 3; ++i)
        ASSERT_EQ(0, am.newGt()[i][0]);

    // the different guy's old index of 0 should now point to 1 in 
    // the merged array
    ASSERT_EQ(1, am.newGt()[3][0]);
}

TEST_F(TestVcfAlleleMerger, deletion) {
    // mock reference sequence with CTCGG starting at 1:38
    MockRef ref("CTGGG", "1", 38);

    vector<Entry> entries;
    // the next 3 are equivalent
    entries.push_back(makeEntry("1", 39, "TG", "T"));
    entries.push_back(makeEntry("1", 40, "GG", "G"));
    entries.push_back(makeEntry("1", 41, "GG", "G"));

    AlleleMerger<MockRef> am(ref, entries);

    // yes, something was merged.
    ASSERT_TRUE(am.merged());

    // make sure the total # of (unique) alts is now 2
    ASSERT_EQ(1, am.mergedAlt().size());
    ASSERT_EQ("TGG", am.mergedAlt()[0]);

    ASSERT_EQ(3, am.newGt().size());

    for (size_t i = 0; i < 3; ++i)
        ASSERT_EQ(0, am.newGt()[i][0]);
}

TEST_F(TestVcfAlleleMerger, snv) {
    // mock reference sequence with CTCGG starting at 1:38
    MockRef ref("CTGGG", "1", 38);

    vector<Entry> entries;
    // the next 3 are equivalent
    entries.push_back(makeEntry("1", 39, "TGC", "TGC"));
    entries.push_back(makeEntry("1", 40, "GG", "GC"));
    entries.push_back(makeEntry("1", 41, "G", "C"));

    AlleleMerger<MockRef> am(ref, entries);

    // yes, something was merged.
    ASSERT_TRUE(am.merged());

    // make sure the total # of (unique) alts is now 2
    ASSERT_EQ(1, am.mergedAlt().size());
    ASSERT_EQ("TGC", am.mergedAlt()[0]);

    ASSERT_EQ(3, am.newGt().size());

    for (size_t i = 0; i < 3; ++i)
        ASSERT_EQ(0, am.newGt()[i][0]);
}

TEST_F(TestVcfAlleleMerger, identical) {
    MockRef ref("CTGGG", "1", 38);

    vector<Entry> entries;
    // the next 3 are equivalent
    entries.push_back(makeEntry("1", 39, "T", "TG"));
    entries.push_back(makeEntry("1", 39, "T", "TG"));

    AlleleMerger<MockRef> am(ref, entries);
    ASSERT_TRUE(am.merged());
    ASSERT_EQ(1, am.mergedAlt().size());
    ASSERT_EQ("TG", am.mergedAlt()[0]);

    ASSERT_EQ(2, am.newGt().size());
    ASSERT_EQ(0, am.newGt()[0][0]);
    ASSERT_EQ(0, am.newGt()[1][0]);
}

TEST_F(TestVcfAlleleMerger, overlapButNoMerge) {
    MockRef ref("CTGGG", "1", 38);

    vector<Entry> entries;
    // the next 3 are equivalent
    entries.push_back(makeEntry("1", 39, "T", "TG"));
    entries.push_back(makeEntry("1", 39, "T", "TC"));

    AlleleMerger<MockRef> am(ref, entries);
    ASSERT_FALSE(am.merged());
}

TEST_F(TestVcfAlleleMerger, mismatchChrom) {
    MockRef ref("CTGGG", "1", 38);

    vector<Entry> entries;
    // the next 3 are equivalent
    entries.push_back(makeEntry("1", 39, "T", "TC"));
    entries.push_back(makeEntry("2", 39, "T", "TC"));

    AlleleMerger<MockRef> am(ref, entries);
    ASSERT_FALSE(am.merged());
}
