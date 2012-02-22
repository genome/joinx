#include "fileformats/vcf/AltNormalizer.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/Fasta.hpp"
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

    string _refFa;
    Fasta _ref;
    Header _header;
};

// single alt cases
TEST_F(TestVcfAltNormalizer, insertion) {
    cout << "   REF: " << _ref.sequence("1", 1, 13) << "\n";
    string ref = _ref.sequence("1", 11, 3);
    ASSERT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "GCGCG");
    AltNormalizer n(&_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    ASSERT_EQ(3, e.pos());
    ASSERT_EQ("T", e.ref());
    ASSERT_EQ(1, e.alt().size());
    ASSERT_EQ("TCG", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, insertionWithSubstitution) {
    string ref = _ref.sequence("1", 11, 3);
    ASSERT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "GAGCG");
    AltNormalizer n(&_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    ASSERT_EQ(11, e.pos());
    ASSERT_EQ("GC", e.ref());
    ASSERT_EQ(1, e.alt().size());
    ASSERT_EQ("GAGC", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, deletion) {
    string ref = _ref.sequence("1", 11, 3);
    ASSERT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "G");
    AltNormalizer n(&_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    ASSERT_EQ(3, e.pos());
    ASSERT_EQ("TCG", e.ref());
    ASSERT_EQ(1, e.alt().size());
    ASSERT_EQ("T", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, deletionWithSubstitution) {
    string ref = _ref.sequence("1", 9, 5);
    ASSERT_EQ("GCGCG", ref);
    Entry e = makeEntry("1", 9, ref, "GAG");
    AltNormalizer n(&_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    ASSERT_EQ(9, e.pos());
    ASSERT_EQ("GCGC", e.ref());
    ASSERT_EQ(1, e.alt().size());
    ASSERT_EQ("GA", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, testSubstitution) {
    string ref = _ref.sequence("1", 9, 1);
    ASSERT_EQ("G", ref);
    Entry e = makeEntry("1", 9, ref, "C");
    AltNormalizer n(&_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    ASSERT_EQ(9, e.pos());
    ASSERT_EQ("G", e.ref());
    ASSERT_EQ(1, e.alt().size());
    ASSERT_EQ("C", e.alt()[0]);
}

TEST_F(TestVcfAltNormalizer, testSubstitutionWithPadding) {
    string ref = _ref.sequence("1", 9, 5);
    ASSERT_EQ("GCGCG", ref);
    Entry e = makeEntry("1", 9, ref, "GCGCA");
    AltNormalizer n(&_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";

    ASSERT_EQ(13, e.pos());
    ASSERT_EQ("G", e.ref());
    ASSERT_EQ(1, e.alt().size());
    ASSERT_EQ("A", e.alt()[0]);
}

// multi alt cases
TEST_F(TestVcfAltNormalizer, insertionAndDeletion) {
    string ref = _ref.sequence("1", 11, 3);
    ASSERT_EQ("GCG", ref);
    Entry e = makeEntry("1", 11, ref, "GCGCG,GC");
    AltNormalizer n(&_ref);
    cout << "BEFORE: " << e << "\n";
    n.normalize(e);
    cout << " AFTER: " << e << "\n";
}
