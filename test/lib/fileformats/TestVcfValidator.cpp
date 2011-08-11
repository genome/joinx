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

TEST(VcfValidator, valid) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_TRUE(v(e, &problems));
    ASSERT_TRUE(problems.empty());
}

TEST(VcfValidator, missingInfo) {
    Header h = makeHeader();
    Validator v(h);

    Entry e("1\t1\t.\tG\tA\t50\tPASS\tNO=BAD;NS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3");
    vector<string> problems;
    ASSERT_FALSE(v(e, &problems));
    ASSERT_EQ(1, problems.size());
    // or some other appropriate msg...
    ASSERT_EQ("Unknown field in info section: 'NO'", problems[0]);
    problems.clear();

    // ... same for info/filters sections
}
