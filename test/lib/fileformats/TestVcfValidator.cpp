#include "fileformats/vcf/Validator.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;

namespace {
    string headerText =
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
        ;

    Header makeHeader() {
        stringstream ss(headerText);
        Header h;
        string line;
        while (getline(ss, line))
            h.add(line);

        return h;
    }
}

TEST(VcfValidator, validLine) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_TRUE(v(e, &problems));
    ASSERT_TRUE(problems.empty());
}

TEST(VcfValidator, validFilterID) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tq10\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_TRUE(v(e, &problems));
    ASSERT_TRUE(problems.empty());
}

TEST(VcfValidator, invalidInfoID) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNO=BAD;NS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    ASSERT_EQ("Unknown field in info section: 'NO'", problems[0]);
    problems.clear();
    // ... same for info/filters sections
}

TEST(VcfValidator, invalidFilterID) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tP1_v2\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    ASSERT_EQ("Unknown field in filter section: 'P1_v2'", problems[0]);
    problems.clear();
}

TEST(VcfValidator, invalidFormatID) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP:R2D2\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    ASSERT_EQ("Unknown field in format section: 'R2D2'", problems[0]);
    problems.clear();
}

TEST(VcfValidator, invalidInfoIntegerType) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNS=3.0;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    ASSERT_EQ("Could not cast: '3.0' as Integer", problems[0]);
    problems.clear();
}

TEST(VcfValidator, invalidInfoFloatType) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNS=3;DP=9;AA=G;AF=95%\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    ASSERT_EQ("Could not cast: '95%' as Float", problems[0]);
    problems.clear();
}

TEST(VcfValidator, invalidFormatIntegerType) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35.3:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    ASSERT_EQ("Could not cast: '35.3' as Integer", problems[0]);
    problems.clear();
}

TEST(VcfValidator, incompletePerSampleGenotypeFields) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    ASSERT_EQ("Not enough genotype fields.", problems[0]);
    problems.clear();
}
