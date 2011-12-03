#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/GenotypeFormatter.hpp"

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

namespace {
    string headerText(
        "##fileformat=VCFv4.1"
        "##fileDate=20090805\n"
        "##source=myImputationProgramV3.1\n"
        "##reference=file:///seq/references/1000GenomesPilot-NCBI36.fasta\n"
        "##contig=<ID=20,length=62435964,assembly=B36,md5=f126cdf8a6e0c7f379d618ff66beb2da,species=\"Homo sapiens\",taxonomy=x>\n"
        "##phasing=partial\n"
        "##INFO=<ID=NS,Number=1,Type=Integer,Description=\"Number of Samples With Data\">\n"
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Total Depth\">\n"
        "##INFO=<ID=AF,Number=A,Type=Float,Description=\"Allele Frequency\">\n"
        "##INFO=<ID=AA,Number=1,Type=String,Description=\"Ancestral Allele\">\n"
        "##INFO=<ID=DB,Number=0,Type=Flag,Description=\"dbSNP membership, build 129\">\n"
        "##INFO=<ID=H2,Number=0,Type=Flag,Description=\"HapMap2 membership\">\n"
        "##FILTER=<ID=q10,Description=\"Quality below 10\">\n"
        "##FILTER=<ID=s50,Description=\"Less than 50% of samples have data\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">\n"
        "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##FORMAT=<ID=HQ,Number=2,Type=Integer,Description=\"Haplotype Quality\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA00001\tNA00002\tNA00003\n"
        );

    string vcfLines =
        "20\t14370\trs6054257\tG\tA\t29\t.\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t.\n"
        "20\t17330\t.\tT\tA\t3\tq10\tAF=0.017;DP=11;NS=3\tGT:GQ:DP:HQ\t0|0:49:3:58,50\t0|1:3:5:65,3\t.\n"
        "20\t1110696\trs6040355\tA\tG,T\t67\tPASS\tAA=T;AF=0.333,0.667;DB;DP=10;NS=2\tGT:GQ:DP:HQ\t1|2:21:6:23,27\t2|1:2:0:18,2\t2/2:35:4\n"
        "20\t1230237\t.\tT\t.\t47\tPASS\tAA=T;DP=13;NS=3\tGT:GQ:DP:HQ\t0|0:54:7:56,60\t0|0:48:4:51,51\t0/0:.:2\n"
        "20\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tAA=G;DP=9;NS=3\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:3:3\n"
        "21\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tAA=G;DP=9;NS=3\t.\n"
        "22\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\t.\t.\n"
        ;

}

class TestVcfGenotypeFormatter : public ::testing::Test {
protected:
    void SetUp() {
        stringstream hdrss(headerText);
        InputStream in("test", hdrss);
        _header = Header::fromStream(in);

        stringstream vcfss(vcfLines);
        string line;
        while (getline(vcfss, line)) {
            Entry e(&_header, line);
            v.push_back(e);
        }
    }

    Header _header;
    vector<Entry> v;
};

TEST_F(TestVcfGenotypeFormatter, renumberGT) {
    // the entry only has A, we're pretending like we've merged and now have C and A
    vector<string> altAlleles;
    altAlleles.push_back("C");
    altAlleles.push_back("A");

    vector<size_t> altAlleleIndices;
    altAlleleIndices.push_back(1); // C becomes 1
    altAlleleIndices.push_back(2); // A becomes 2

    GenotypeFormatter fmt(&_header, altAlleles);
    // The GT for sample 2 (index 1) was 1|0, referring to A|G.
    // It should get updated to 2|0 since we put C in front of A in the
    // alt allele array.
    ASSERT_EQ("2|0", fmt.renumberGT(&v[0], 1, altAlleleIndices));

    // no GT data for sample 3 (index 2), should still return empty
    ASSERT_EQ(".", fmt.renumberGT(&v[0], 2, altAlleleIndices));
}

TEST_F(TestVcfGenotypeFormatter, areGenotypesDisjoint) {
    ASSERT_TRUE(GenotypeFormatter::areGenotypesDisjoint("0|0", "2|2"));
    ASSERT_TRUE(GenotypeFormatter::areGenotypesDisjoint("0|1", "2|2"));
    ASSERT_TRUE(GenotypeFormatter::areGenotypesDisjoint("1|0", "2|2"));
    ASSERT_TRUE(GenotypeFormatter::areGenotypesDisjoint("1|1", "2|2"));
    ASSERT_TRUE(GenotypeFormatter::areGenotypesDisjoint("0|0|1", "2|2"));

    ASSERT_FALSE(GenotypeFormatter::areGenotypesDisjoint("1|2", "2|2"));
    ASSERT_FALSE(GenotypeFormatter::areGenotypesDisjoint("2|1", "2|2"));
    ASSERT_FALSE(GenotypeFormatter::areGenotypesDisjoint("2|2", "2|2"));
    ASSERT_FALSE(GenotypeFormatter::areGenotypesDisjoint("0|0|1", "2|2|2|0"));
}

TEST_F(TestVcfGenotypeFormatter, mergeIncompatibleGenotypes) {
    vector<string> altAlleles;
    altAlleles.push_back("A");
    string mergeLines[] = {
        // the GT data is disjoint (no overlap between 0|0 and 1|1, we expect an error about this
        "20\t14370\trs6054257\tG\tA\t29\t.\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t0|0:.:1:1,1\t.\t.",
        "20\t14370\trs6054257\tG\tA\t29\t.\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t1|1:2:2:2,2\t.\t."
    };

    vector<size_t> altAlleleIndices;
    altAlleleIndices.push_back(0);

    Entry primary;
    Entry secondary;
    Entry::parseLine(&_header, mergeLines[0], primary);
    Entry::parseLine(&_header, mergeLines[1], secondary);

    GenotypeFormatter fmt(&_header, altAlleles);
    vector<CustomValue> previousValues = *primary.sampleData(0);
    ASSERT_THROW(
        fmt.merge(false, previousValues, secondary.formatDescription(), &secondary, 0, altAlleleIndices),
        runtime_error
        );
}

TEST_F(TestVcfGenotypeFormatter, merge) {
    vector<string> altAlleles;
    altAlleles.push_back("A");
    string mergeLines[] = {
        "20\t14370\trs6054257\tG\tA\t29\t.\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t0|1:.:1:1,1\t.\t.",
        "20\t14370\trs6054257\tG\tA\t29\t.\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t1|1:2:.:2,2\t.\t."
    };

    vector<size_t> altAlleleIndices;
    altAlleleIndices.push_back(0);

    Entry primary;
    Entry secondary;
    Entry::parseLine(&_header, mergeLines[0], primary);
    Entry::parseLine(&_header, mergeLines[1], secondary);

    GenotypeFormatter fmt(&_header, altAlleles);
    vector<CustomValue> previousValues = *primary.sampleData(0);
    fmt.merge(
        false, // do not override values that are already set
        previousValues,
        secondary.formatDescription(),
        &secondary, // entry to take data from
        0, // take data from sample #0
        altAlleleIndices
        );

    // note: the primary sample has no value for GQ, while the secondary does. we indicated
    // that the secondary sample should only fill in gaps in the primary data, so we expect
    // the results to look like the primary values with the GQ value from the secondary (2)
    const string* gt = previousValues[0].get<string>(0);
    ASSERT_TRUE(gt);
    ASSERT_EQ("0|1", *gt);

    // note: this value comes from the secondary entry
    const int64_t* gq = previousValues[1].get<int64_t>(0);
    ASSERT_TRUE(gq);
    ASSERT_EQ(2, *gq);

    const int64_t* dp = previousValues[2].get<int64_t>(0);
    ASSERT_TRUE(dp);
    ASSERT_EQ(1, *dp);

    const int64_t* hq1 = previousValues[3].get<int64_t>(0);
    ASSERT_TRUE(hq1);
    ASSERT_EQ(1, *hq1);

    const int64_t* hq2 = previousValues[3].get<int64_t>(1);
    ASSERT_TRUE(hq2);
    ASSERT_EQ(1, *hq2);

    // now we say DO override previous values
    fmt.merge(true, previousValues, secondary.formatDescription(), &secondary, 0, altAlleleIndices);

    gt = previousValues[0].get<string>(0);
    ASSERT_TRUE(gt);
    ASSERT_EQ("1|1", *gt);

    gq = previousValues[1].get<int64_t>(0);
    ASSERT_TRUE(gq);
    ASSERT_EQ(2, *gq);

    // secondary sample had no depth data, data from primary should remain
    dp = previousValues[2].get<int64_t>(0);
    ASSERT_TRUE(dp);
    ASSERT_EQ(1, *dp);

    hq1 = previousValues[3].get<int64_t>(0);
    ASSERT_TRUE(hq1);
    ASSERT_EQ(2, *hq1);

    hq2 = previousValues[3].get<int64_t>(1);
    ASSERT_TRUE(hq2);
    ASSERT_EQ(2, *hq2);
}
