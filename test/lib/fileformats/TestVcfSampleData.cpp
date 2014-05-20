#include "fileformats/vcf/SampleData.hpp"

#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"

#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>

#include <cstdlib>
#include <string>

using boost::assign::list_of;

namespace {
    std::string headerText(
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
        "##FORMAT=<ID=FT,Number=1,Type=String,Description=\"Sample Filtering\">\n"
        "##FORMAT=<ID=FPV,Number=1,Type=Float,Description=\"Floating point value\">\n"
        "##FORMAT=<ID=VLSL,Number=.,Type=String,Description=\"Variable length string list\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\tS2\tS3\tS4\tS5\n"
        );
}

class TestVcfSampleData : public ::testing::Test {
public:
    void SetUp() {
        header = Vcf::Header::fromString(headerText);
        format = "GT:GQ:DP:HQ:FPV:VLSL";
        oneSample = "0/1:34:120:31:1e-6:A,B,C";
    }

protected:
    std::string format;
    std::string oneSample;
    Vcf::Header header;
};

TEST_F(TestVcfSampleData, removeFilteredWhitelist) {
    std::set<std::string> keep{"OK", "ForcedGenotype"};
    std::string text =
        "GT:FT"
        "\t0/1:."
        "\t0/1:BAD"
        "\t0/1:OK"
        "\t0/1:PASS"
        "\t0/1:ForcedGenotype"
        ;

    Vcf::SampleData sd(&header, text);
    for (int i = 0; i < 5; ++i) {
        ASSERT_FALSE(0 == sd.get(i));
        EXPECT_FALSE(sd.get(i)->empty());
    }

    sd.removeFilteredWhitelist(keep);
    EXPECT_TRUE(0 != sd.get(0) && !sd.get(0)->empty());
    EXPECT_TRUE(0 == sd.get(1) || sd.get(1)->empty());
    EXPECT_TRUE(0 != sd.get(2) && !sd.get(2)->empty());
    EXPECT_TRUE(0 != sd.get(3) && !sd.get(3)->empty());
    EXPECT_TRUE(0 != sd.get(4) && !sd.get(4)->empty());
}

TEST_F(TestVcfSampleData, parse) {
    std::string txt = format + "\t" + oneSample;
    Vcf::SampleData sd(&header, txt);

    std::vector<std::string> expectedFormat = list_of
        ("GT")("GQ")("DP")("HQ")("FPV")("VLSL")
        ;

    Vcf::CustomValue const* gt = sd.get(0, "GT");
    ASSERT_TRUE(gt);

    std::string const* gtString = gt->get<std::string>(0);
    EXPECT_EQ("0/1", *gtString);

    ASSERT_EQ(expectedFormat.size(), sd.format().size());
    for (size_t i = 0; i < expectedFormat.size(); ++i) {
        EXPECT_EQ(expectedFormat[i], sd.format()[i]->id());
    }
}

TEST_F(TestVcfSampleData, addFilterReflectedSamples) {
    std::string txt = format + "\t" + oneSample;

    header.mirrorSample("S1", "S1-COPY");
    auto mainIdx = header.sampleIndex("S1");
    auto copyIdx = header.sampleIndex("S1-COPY");

    Vcf::SampleData sd(&header, txt);

    EXPECT_EQ(2u, sd.samplesWithData());

    EXPECT_FALSE(sd.isSampleFiltered(mainIdx));
    EXPECT_FALSE(sd.isSampleFiltered(copyIdx));

    sd.addFilter(mainIdx, "HATE");
    std::string filterName;
    EXPECT_TRUE(sd.isSampleFiltered(mainIdx, &filterName));
    EXPECT_EQ("HATE", filterName);
    EXPECT_FALSE(sd.isSampleFiltered(copyIdx));
}


TEST_F(TestVcfSampleData, addFilterReflectingSamples) {
    std::string txt = format + "\t" + oneSample;

    header.mirrorSample("S1", "S1-COPY");
    auto mainIdx = header.sampleIndex("S1");
    auto copyIdx = header.sampleIndex("S1-COPY");

    Vcf::SampleData sd(&header, txt);

    EXPECT_EQ(2u, sd.samplesWithData());

    EXPECT_FALSE(sd.isSampleFiltered(mainIdx));
    EXPECT_FALSE(sd.isSampleFiltered(copyIdx));

    sd.addFilter(copyIdx, "HATE");
    std::string filterName;
    EXPECT_TRUE(sd.isSampleFiltered(copyIdx, &filterName));
    EXPECT_EQ("HATE", filterName);
    EXPECT_FALSE(sd.isSampleFiltered(mainIdx));
}
