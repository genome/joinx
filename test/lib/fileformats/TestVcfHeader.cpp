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
        "##fileformat=VCFv4.1\n"
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
        ;

    string conflictingHeader =
        "##fileformat=VCFv4.1\n"
        "##fileDate=20090805\n"
        "##source=myImputationProgramV3.1\n"
        "##reference=file:///seq/references/1000GenomesPilot-NCBI36.fasta\n"
        "##contig=<ID=20,length=62435964,assembly=B36,md5=f126cdf8a6e0c7f379d618ff66beb2da,species=\"Homo sapiens\",taxonomy=x>\n"
        "##phasing=partial\n"
        "##INFO=<ID=NS,Number=1,Type=Integer,Description=\"Number of Samples With Data\">\n"
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Total Depth\">\n"
        "##INFO=<ID=AF,Number=G,Type=Float,Description=\"Allele Frequency\">\n"
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
        ;

    string differentData =
        "##fileformat=VCFv4.1\n"
        "##fileDate=20090805\n"
        "##source=myImputationProgramV3.1\n"
        "##reference=file:///seq/references/1000GenomesPilot-NCBI36.fasta\n"
        "##contig=<ID=20,length=62435964,assembly=B36,md5=f126cdf8a6e0c7f379d618ff66beb2da,species=\"Homo sapiens\",taxonomy=x>\n"
        "##phasing=partial\n"
        "##INFO=<ID=NW,Number=.,Type=Integer,Description=\"Extra field to test merge\">\n"
        "##INFO=<ID=NS,Number=1,Type=Integer,Description=\"Number of Samples With Data\">\n"
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Total Depth\">\n"
        "##INFO=<ID=AF,Number=A,Type=Float,Description=\"Allele Frequency\">\n"
        "##INFO=<ID=DB,Number=0,Type=Flag,Description=\"dbSNP membership, build 129\">\n"
        "##INFO=<ID=H2,Number=0,Type=Flag,Description=\"HapMap2 membership\">\n"
        "##FILTER=<ID=q10,Description=\"Quality below 10\">\n"
        "##FILTER=<ID=s50,Description=\"Less than 50% of samples have data\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##FORMAT=<ID=HQ,Number=2,Type=Integer,Description=\"Haplotype Quality\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tMA00001\tMA00002\tMA00003\n"
        ;

    string expectedMerged = 
        "##fileformat=VCFv4.1\n"
        "##fileDate=20090805\n"
        "##source=myImputationProgramV3.1\n"
        "##reference=file:///seq/references/1000GenomesPilot-NCBI36.fasta\n"
        "##contig=<ID=20,length=62435964,assembly=B36,md5=f126cdf8a6e0c7f379d618ff66beb2da,species=\"Homo sapiens\",taxonomy=x>\n"
        "##phasing=partial\n"
        "##INFO=<ID=NW,Number=.,Type=Integer,Description=\"Extra field to test merge\">\n"
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
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA00001\tNA00002\tNA00003\tMA00001\tMA00002\tMA00003\n"
        ;



    Header parse(const std::string& s) {
        stringstream ss(s);
        Header h;
        string line;
        while (getline(ss, line))
            h.add(line);
        return h;
    }
}

TEST(VcfHeader, parse) {
    Header h = parse(headerText);
    const set<string>& categories = h.categories();
    ASSERT_TRUE(categories.find("contig") != categories.end());
    ASSERT_EQ(4, categories.size());
    ASSERT_TRUE(categories.find("INFO") != categories.end());
    ASSERT_TRUE(categories.find("FILTER") != categories.end());
    ASSERT_TRUE(categories.find("FORMAT") != categories.end());

    const Header::Category& info = h.category("INFO");
    ASSERT_EQ(6, info.size());

    ASSERT_EQ("NS", info[0]["ID"]);
    ASSERT_EQ("1", info[0]["Number"]);
    ASSERT_EQ("Integer", info[0]["Type"]);
    ASSERT_EQ("\"Number of Samples With Data\"", info[0]["Description"]);

    ASSERT_EQ("H2", info[5]["ID"]);
    ASSERT_EQ("0", info[5]["Number"]);
    ASSERT_EQ("Flag", info[5]["Type"]);
    ASSERT_EQ("\"HapMap2 membership\"", info[5]["Description"]);

    ASSERT_EQ(3, h.sampleNames().size());
    ASSERT_EQ("NA00001", h.sampleNames()[0]);
    ASSERT_EQ("NA00002", h.sampleNames()[1]);
    ASSERT_EQ("NA00003", h.sampleNames()[2]);
}

TEST(VcfHeader, merge) {
    Header h1 = parse(headerText);
    Header h1copy = parse(headerText);
    Header conflict = parse(conflictingHeader);
    Header different = parse(differentData);
    
    // throw due to sample conflict
    ASSERT_THROW(h1.merge(h1copy), runtime_error);
    ASSERT_THROW(h1.merge(conflict), runtime_error);
    ASSERT_NO_THROW(h1.merge(different));
    stringstream ss;
    ss << h1;
    const Map* item = h1.categoryItem("INFO", "NW");
    ASSERT_TRUE(item);
    ASSERT_EQ("\"Extra field to test merge\"", (*item)["Description"]);
    item = h1.categoryItem("INFO", "AA");
    ASSERT_TRUE(item);
    ASSERT_EQ("\"Ancestral Allele\"", (*item)["Description"]);
    ASSERT_EQ(6, h1.sampleNames().size());
    ASSERT_EQ("NA00001", h1.sampleNames()[0]);
    ASSERT_EQ("NA00002", h1.sampleNames()[1]);
    ASSERT_EQ("NA00003", h1.sampleNames()[2]);
    ASSERT_EQ("MA00001", h1.sampleNames()[3]);
    ASSERT_EQ("MA00002", h1.sampleNames()[4]);
    ASSERT_EQ("MA00003", h1.sampleNames()[5]);
}

TEST(VcfHeader, sampleIndex) {
    Header h = parse(headerText);
    ASSERT_EQ(0, h.sampleIndex("NA00001"));
    ASSERT_EQ(1, h.sampleIndex("NA00002"));
    ASSERT_EQ(2, h.sampleIndex("NA00003"));
    ASSERT_THROW(h.sampleIndex("NA00004"), runtime_error);
    ASSERT_THROW(h.sampleIndex("hi"), runtime_error);
}


TEST(VcfHeader, toStream) {
    Header h = parse(headerText);
    stringstream ss;
    ss << h;
    ASSERT_EQ(headerText, ss.str());
}
