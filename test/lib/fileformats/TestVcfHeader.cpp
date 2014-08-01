#include "fileformats/vcf/Header.hpp"

#include <gtest/gtest.h>

#include <boost/unordered_map.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
        "##fileDate=20090806\n"
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
        return Header::fromStream(ss);
    }
}

TEST(VcfHeader, parse) {
    Header h = parse(headerText);

    ASSERT_EQ(6u, h.infoTypes().size());
    ASSERT_EQ(4u, h.formatTypes().size());

    ASSERT_EQ(3u, h.sampleNames().size());
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
    ASSERT_THROW(h1.merge(h1copy, false), runtime_error);
    ASSERT_THROW(h1.merge(conflict, false), runtime_error);
    ASSERT_NO_THROW(h1.merge(different));
    ASSERT_EQ(6u, h1.sampleNames().size());
    ASSERT_EQ("NA00001", h1.sampleNames()[0]);
    ASSERT_EQ("NA00002", h1.sampleNames()[1]);
    ASSERT_EQ("NA00003", h1.sampleNames()[2]);
    ASSERT_EQ("MA00001", h1.sampleNames()[3]);
    ASSERT_EQ("MA00002", h1.sampleNames()[4]);
    ASSERT_EQ("MA00003", h1.sampleNames()[5]);

    // doesn't throw if we allow duplicate sample names
    ASSERT_NO_THROW(h1.merge(h1copy, true));
    // make sure we didn't get duplicate names
    ASSERT_EQ(6u, h1.sampleNames().size());
    ASSERT_EQ("NA00001", h1.sampleNames()[0]);
    ASSERT_EQ("NA00002", h1.sampleNames()[1]);
    ASSERT_EQ("NA00003", h1.sampleNames()[2]);
    ASSERT_EQ("MA00001", h1.sampleNames()[3]);
    ASSERT_EQ("MA00002", h1.sampleNames()[4]);
    ASSERT_EQ("MA00003", h1.sampleNames()[5]);

    unsigned dateCount(0);
    const vector<Header::RawLine>& metaInfo = h1.metaInfoLines();
    string date;
    for (auto i = metaInfo.begin(); i != metaInfo.end(); ++i) {
        if (i->first == "fileDate") {
            ++dateCount;
            date = i->second;
        }
    }
    ASSERT_EQ(1u, dateCount);
    char dateStr[32] = {0};
    time_t now = time(NULL);
    strftime(dateStr, sizeof(dateStr), "%Y%m%d", localtime(&now));
    ASSERT_EQ(dateStr, date);

    // conflicting info field still throws an error even allowing duplicate samples
    ASSERT_THROW(h1.merge(conflict, true), runtime_error);
}

TEST(VcfHeader, sampleIndex) {
    Header h = parse(headerText);
    ASSERT_EQ(0u, h.sampleIndex("NA00001"));
    ASSERT_EQ(1u, h.sampleIndex("NA00002"));
    ASSERT_EQ(2u, h.sampleIndex("NA00003"));
    ASSERT_THROW(h.sampleIndex("NA00004"), runtime_error);
    ASSERT_THROW(h.sampleIndex("hi"), runtime_error);
}


TEST(VcfHeader, toStream) {
    Header h = parse(headerText);
    stringstream ss;
    ss << h;
    ASSERT_EQ(headerText, ss.str());
}

TEST(VcfHeader, addFilter) {
    Header h = parse(headerText);
    h.addFilter("FOO12", "Test filter");
    const auto& filters = h.filters();
    auto iter = filters.find("FOO12");
    ASSERT_FALSE(iter == filters.end());
    ASSERT_EQ("\"Test filter\"", iter->second);
    stringstream ss;
    ss << h;
    string expected = "##FILTER=<ID=FOO12,Description=\"Test filter\">";
    ASSERT_NE(string::npos, ss.str().find(expected));
}

TEST(VcfHeader, addInfoType) {
    Header h = parse(headerText);
    CustomType t(
        "SUPERTYPE",
        CustomType::FIXED_SIZE,
        1,
        CustomType::STRING,
        "A type"
        );

    h.addInfoType(t);
    auto const* fetch = h.infoType("SUPERTYPE");
    ASSERT_TRUE(fetch);
    ASSERT_EQ("SUPERTYPE", fetch->id());
    ASSERT_EQ(CustomType::FIXED_SIZE, fetch->numberType());
    ASSERT_EQ(1u, fetch->number());
    ASSERT_EQ(CustomType::STRING, fetch->type());
    ASSERT_EQ("A type", fetch->description());

    stringstream ss;
    ss << h;
    string expected = "##INFO=<ID=SUPERTYPE,Number=1,Type=String,Description=\"A type\">";
    ASSERT_NE(string::npos, ss.str().find(expected));
}

TEST(VcfHeader, sampleMirroring) {
    Header h = parse(headerText);
    EXPECT_EQ(3u, h.sampleCount());
    EXPECT_FALSE(h.isReflected(0));
    EXPECT_FALSE(h.isReflected(1));
    EXPECT_FALSE(h.isReflected(2));

    EXPECT_FALSE(h.isReflection(0));
    EXPECT_FALSE(h.isReflection(1));
    EXPECT_FALSE(h.isReflection(2));

    h.mirrorSample("NA00001", "one");

    EXPECT_EQ(4u, h.sampleCount());
    EXPECT_EQ("one", h.sampleNames()[3]);
    EXPECT_TRUE(h.isReflected(0));
    EXPECT_FALSE(h.isReflected(1));
    EXPECT_FALSE(h.isReflected(2));
    EXPECT_FALSE(h.isReflected(3));

    EXPECT_FALSE(h.isReflection(0));
    EXPECT_FALSE(h.isReflection(1));
    EXPECT_FALSE(h.isReflection(2));
    EXPECT_TRUE(h.isReflection(3));
}

TEST(VcfHeader, renameSamples) {
    std::stringstream hdrss(
        "##fileformat=VCFv4.2\n"
        "##SAMPLE=<ID=S1,Data=x>\n"
        "##SAMPLE=<ID=S2,Data=y>\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\tS2\n"
        );

    auto h = Header::fromStream(hdrss);
    ASSERT_EQ(2u, h.sampleNames().size());
    EXPECT_EQ("S1", h.sampleNames()[0]);
    EXPECT_EQ("S2", h.sampleNames()[1]);

    EXPECT_THROW(h.sampleIndex("SAMPLE1"), SampleNotFoundError);
    EXPECT_EQ(0u, h.sampleIndex("S1"));
    EXPECT_EQ(1u, h.sampleIndex("S2"));

    SampleTag const* tag = h.sampleTag("S1");
    ASSERT_TRUE(tag);
    EXPECT_EQ("S1", *tag->get("ID"));
    EXPECT_EQ("x", *tag->get("Data"));

    tag = h.sampleTag("S2");
    ASSERT_TRUE(tag);
    EXPECT_EQ("S2", *tag->get("ID"));
    EXPECT_EQ("y", *tag->get("Data"));

    boost::unordered_map<std::string, std::string> renames{
        {"S1", "SAMPLE1"}
        };

    h.renameSamples(renames);
    ASSERT_EQ(2u, h.sampleNames().size());
    EXPECT_EQ("SAMPLE1", h.sampleNames()[0]);
    EXPECT_EQ("S2", h.sampleNames()[1]);

    EXPECT_THROW(h.sampleIndex("S1"), SampleNotFoundError);
    EXPECT_EQ(0u, h.sampleIndex("SAMPLE1"));
    EXPECT_EQ(1u, h.sampleIndex("S2"));

    tag = h.sampleTag("S1");
    ASSERT_FALSE(tag);

    tag = h.sampleTag("SAMPLE1");
    ASSERT_TRUE(tag);

    EXPECT_EQ("SAMPLE1", *tag->get("ID"));
    EXPECT_EQ("x", *tag->get("Data"));

    tag = h.sampleTag("S2");
    ASSERT_TRUE(tag);
    EXPECT_EQ("S2", *tag->get("ID"));
    EXPECT_EQ("y", *tag->get("Data"));
}
