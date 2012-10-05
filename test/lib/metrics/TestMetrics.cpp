#include "metrics/Metrics.hpp"
#include "common/Sequence.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace Vcf;
using namespace std::placeholders;
using namespace std;

namespace {
    string headerText(
        "##fileformat=VCFv4.1\n"
        "##phasing=partial\n"
        "##INFO=<ID=DBSNP,Number=A,Type=Integer,Description=\"dbSNP membership, build 135\">\n"
        "##INFO=<ID=TG,Number=A,Type=Integer,Description=\"Thousand genomes wgs membership\">\n"
        "##FILTER=<ID=DIE,Description=\"Eat flaming death\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "##FORMAT=<ID=FT,Number=1,Type=String,Description=\"Sample Filtering\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\t"
        "S1\tS2\tS3\tS4\tS5\tS6\tS7\tS8\tS9\tS10\tS11\tS12\tS13\tS14\tS15\tS16"
        );


    // entry for testing per-entry metrics
    string entries[] = {
        // ENTRY 1
        "1\t20\t.\tA\tC,T\t.\t.\t.\tGT:FT\t"
        // here come the genotypes
        "0/0:PASS\t" // 1x ref

        "1/0:PASS\t" // 4x 0/1 or 1/0, with 1 filtered
        "0/1:DIE\t"
        "1/0:PASS\t"
        "0/1:PASS\t"

        "1/1:PASS\t" // 1x 1/1
        "0/2:PASS\t" // 2x 0/2 or 2/0
        "2/0:PASS\t"
        "2|1:PASS\t" // 1x
        "2/2:PASS\t" // 1x

        // ,
        // ENTRY 2
    };
}

// Test fixture for normal tests
class TestMetrics : public ::testing::Test {
protected:
    void SetUp() {
        stringstream hdrss(headerText);
        InputStream in("test", hdrss);
        _header = Header::fromStream(in);
        _novelIndicators.push_back("DBSNP");
        _novelIndicators.push_back("TG");
        int n_entries = sizeof(entries)/sizeof(entries[0]);
        for (int i = 0; i < n_entries; ++i) {
            Entry e;
            Vcf::Entry::parseLine(&_header, entries[i], e);
            _entries.push_back(e);
            _metrics.emplace_back(new Metrics::EntryMetrics(e, _novelIndicators));
        }
    }

    Entry makeEntry(string chrom, int64_t pos, string const& ref, string const& alt) {
        stringstream ss;
        ss << chrom << "\t" << pos << "\t.\t" << ref << "\t" << alt << "\t.\t.\t.";
        return Entry(&_header, ss.str());
    }

    Header _header;
    vector<string> _novelIndicators;
    vector<Entry> _entries;
    vector<unique_ptr<Metrics::EntryMetrics>> _metrics;
};

// Parameterized test fixture for mutation spectrum test of doom.
// (parameterized by reference base)
class TestMetricsParamByRef: public ::testing::TestWithParam<const char*> {
protected:
    void SetUp() {
        stringstream hdrss(headerText);
        InputStream in("test", hdrss);
        _header = Header::fromStream(in);
        _novelIndicators.push_back("DBSNP");
    }

    Header _header;
    vector<string> _novelIndicators;
};

// NOTE: The expected values in most of these tests are based on
// entry[0] defined near the top of this module.
TEST_F(TestMetrics, genotypeDistribution) {
    auto dist = _metrics[0]->genotypeDistribution();

    ASSERT_EQ(6, dist.size());
    ASSERT_EQ(1, dist[GenotypeCall("0/0")]);
    ASSERT_EQ(0, dist[GenotypeCall("0|0")]); // make sure phased doesn't match unphased

    ASSERT_EQ(3, dist[GenotypeCall("0/1")]);
    ASSERT_EQ(3, dist[GenotypeCall("1/0")]);
    ASSERT_EQ(0, dist[GenotypeCall("0|1")]);
    ASSERT_EQ(0, dist[GenotypeCall("1|0")]);

    ASSERT_EQ(1, dist[GenotypeCall("1/1")]);
    ASSERT_EQ(0, dist[GenotypeCall("1|1")]);

    ASSERT_EQ(2, dist[GenotypeCall("2/0")]);
    ASSERT_EQ(2, dist[GenotypeCall("0/2")]);
    ASSERT_EQ(0, dist[GenotypeCall("2|0")]);
    ASSERT_EQ(0, dist[GenotypeCall("0|2")]);

    ASSERT_EQ(0, dist[GenotypeCall("1|2")]); // ordering matters for phased
    ASSERT_EQ(1, dist[GenotypeCall("2|1")]); // <-
    ASSERT_EQ(0, dist[GenotypeCall("2/1")]); // make sure unphased doesn't match phased
    ASSERT_EQ(0, dist[GenotypeCall("1/2")]);

    ASSERT_EQ(1, dist[GenotypeCall("2/2")]);
    ASSERT_EQ(0, dist[GenotypeCall("2|2")]);
}

TEST_F(TestMetrics, allelicDistribution) {
    // REF = A, ALT = C,T
    auto const& dist = _metrics[0]->allelicDistribution();
    ASSERT_EQ(3, dist.size());

    // How many times did we see the reference (GT 0)
    ASSERT_EQ(7, dist[0]); // there were 8 but 1 was filtered
    // How many times did we see C (GT 1)
    ASSERT_EQ(6, dist[1]);
    // How many times did we see T (GT 2)
    ASSERT_EQ(5, dist[2]);
}

TEST_F(TestMetrics, allelicDistributionBySample) {
    auto const& dist = _metrics[0]->allelicDistributionBySample();

    ASSERT_EQ(3, dist.size());

    // How many samples have an A (GT 0)
    ASSERT_EQ(6, dist[0]); // there were 7 but 1 was filtered
    // How many have a C (GT 1)
    ASSERT_EQ(5, dist[1]);
    // How many have a T (GT 2)
    ASSERT_EQ(4, dist[2]);

}

TEST_F(TestMetrics, minorAlleleFrequency) {
    // we have diploid 10 samples with 1 filtered for 18 total alleles
    // the minor allele is 2, which shows up 5 times in the data.
    // (see the definition of entries[0])
    ASSERT_NEAR(5.0/18.0, _metrics[0]->minorAlleleFrequency(), 1e-14);
}

// The next test probably warrants an explanation.
//
// The test is parameterized by the reference allele to use.
// Given ref, we generate a vcf entry with all 3 non-reference
// alleles as alts. Then, we iteratively tack on all possible
// genotypes and test that we get the expected result at each step.
//
// For example:
//
// if ref = 'A', then alts = 'C,G,T'.
//
// the genotypes at each iteration are:
//
// iter  0: 0/0
// iter  1: 0/0,0/1
// iter  2: 0/0,0/1,0/2
// iter  3: 0/0,0/1,0/2,0/3
// iter  4: 0/0,0/1,0/2,0/3,1/0
// iter  5: 0/0,0/1,0/2,0/3,1/0,1/1
// ...
// iter 16: 0/0,0/1,...,3/2,3/3
//
// each time we add a genotype, we increment a counter for how many
// times we have seen each variant allele in a call (making sure not
// to double count homozygous mutations) and use that to check that
// EntryMetrics::calculateMutationSpectrum is doing the right thing.
TEST_P(TestMetricsParamByRef, mutationSpectrum) {

    // forward and reverse reference base
    string fwdRef = GetParam();
    string revRef = Sequence::reverseComplement(fwdRef);
    string fwdBases("ACGT");
    string revBases("TGCA");

    // move reference base to the front of the bases array so that
    // bases[0] = ref, bases[1..3] = alts
    size_t refPos = fwdBases.find(fwdRef);
    if (refPos != 0) {
        swap(fwdBases[0], fwdBases[refPos]);
        swap(revBases[0], revBases[refPos]);
    }

    // build the vcf entry as a string
    string line =
        "1\t20\t.\t" + fwdRef + "\t";

    // add in the alts (making sure to skip the reference base
    for (int i = 1; i < 4; ++i) {
        if (i > 1)
            line += ',';
        line += fwdBases[i];
    }
    line += "\t.\t.\t.\tGT:FT";

    // calculateMutationSpectrum reverse complements if ref = G or T,
    // we should do the same
    string const& bases = (fwdRef == "G" || fwdRef == "T") ? revBases : fwdBases;
    char ref = (fwdRef == "G" || fwdRef == "T") ? revRef[0] : fwdRef[0];

    stringstream gts;
    // generate all 16 possible genotypes
    int expected[4] = {0};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            ++expected[i];
            if (j != i) // don't double count homozygous case
                ++expected[j];

            gts << "\t" << i << "/" << j << ":PASS";
            string entryString(line + gts.str());
            Entry entry(&_header, entryString);
            Metrics::EntryMetrics metrics(entry, _novelIndicators);
            auto const& spectrum = metrics.mutationSpectrum();
            auto const& singletonSpectrum = metrics.singletonMutationSpectrum();

            for (int k = 0; k < j; ++k) {
                if (fwdBases[k] == fwdRef[0])
                    continue;

                if (expected[k] == 1) {
                    EXPECT_EQ(0, spectrum(ref, bases[k]))
                        << "At i,j,k = " << i << ", " << j << ", " << k
                        << "\n" << spectrum;
                    EXPECT_EQ(1, singletonSpectrum(ref, bases[k]))
                        << "At i,j,k = " << i << ", " << j << ", " << k
                        << "\n" << singletonSpectrum;
                } else {
                    EXPECT_EQ(0, singletonSpectrum(ref, bases[k]))
                        << "At i,j,k = " << i << ", " << j << ", " << k
                        << "\n" << singletonSpectrum;
                    EXPECT_EQ(expected[k], spectrum(ref, bases[k]))
                        << "At i,j,k = " << i << ", " << j << ", " << k
                        << "\n" << spectrum;
                }
            }

        }
    }
}

INSTANTIATE_TEST_CASE_P(
    ReferenceVariations,
    TestMetricsParamByRef,
    ::testing::Values("A", "C", "G", "T")
);

TEST_F(TestMetrics, identifyNovelAlleles) {
    // need 2 entries to get all 4 cases
    string line1 = "1\t20\t.\tA\tC,G,T\t.\t.\tDBSNP=.,.,1;TG=.,1,.\tGT:FT";
    string line2 = "1\t20\t.\tA\tC\t.\t.\tDBSNP=1;TG=1\tGT:FT";
    Entry e1(&_header, line1);
    Entry e2(&_header, line2);

    Metrics::EntryMetrics em1(e1, _novelIndicators);
    Metrics::EntryMetrics em2(e2, _novelIndicators);

    auto const& novel1 = em1.novelStatusByAlt();
    auto const& novel2 = em2.novelStatusByAlt();

    // did we get all 4 cases?
    ASSERT_EQ(3, novel1.size());
    ASSERT_EQ(1, novel2.size());

    //                  membership: dbsnp 1kg
    ASSERT_TRUE(novel1[0]);  //     0 0
    ASSERT_FALSE(novel1[1]); //     0 1
    ASSERT_FALSE(novel1[2]); //     1 0
    ASSERT_FALSE(novel2[0]); //     1 1
}
