#include "fileformats/vcf/Header.hpp"

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
}

TEST(VcfHeader, parse) {
    stringstream ss(headerText);
    Header h;
    string line;
    while (getline(ss, line))
        h.add(line);

    const set<string>& categories = h.categories();
    ASSERT_TRUE(categories.find("contig") != categories.end());
    ASSERT_EQ(4, categories.size());
    ASSERT_TRUE(categories.find("INFO") != categories.end());
    ASSERT_TRUE(categories.find("FILTER") != categories.end());
    ASSERT_TRUE(categories.find("FORMAT") != categories.end());

    const Header::Category* info = h.category("INFO");
    ASSERT_FALSE(info == NULL);
    ASSERT_EQ(6, info->size());

    ASSERT_EQ("NS", (*info)[0]["ID"]);
    ASSERT_EQ("1", (*info)[0]["Number"]);
    ASSERT_EQ("Integer", (*info)[0]["Type"]);
    ASSERT_EQ("\"Number of Samples With Data\"", (*info)[0]["Description"]);

    ASSERT_EQ("H2", (*info)[5]["ID"]);
    ASSERT_EQ("0", (*info)[5]["Number"]);
    ASSERT_EQ("Flag", (*info)[5]["Type"]);
    ASSERT_EQ("\"HapMap2 membership\"", (*info)[5]["Description"]);
}

TEST(VcfHeader, toStream) {
    stringstream ss(headerText);
    Header h;
    string line;
    while (getline(ss, line))
        h.add(line);
    ss << h;
    ASSERT_EQ(headerText, ss.str());
}
