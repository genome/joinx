#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/VariantAdaptor.hpp"
#include "io/InputStream.hpp"

#include <boost/assign/list_of.hpp>

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using boost::assign::list_of;
using namespace Vcf;
using namespace std;

namespace {
    string headerText(
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
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA00001\tNA00002\tNA00003\n"
        );

    // this header is identical save for the sample ids, it prepends one called 'EXTRA' and
    // reverses the order of NA1-3
    string header2Text(
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
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tEXTRA\tNA00003\tNA00002\tNA00001\n"
        );

    string vcfLines =
        "20\t14370\trs6054257\tG\tA\t29\t.\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t1/1:43:5:.,.\n"
        "20\t17330\t.\tT\tA\t3\tq10\tAF=0.017;DP=11;NS=3\tGT:GQ:DP:HQ:FT\t0|0:49:3:58,50:.\t0|1:3:5:65,3:PASS\t.\n"
        "20\t1110696\trs6040355\tA\tG,T\t67\tPASS\tAA=T;AF=0.333,0.667;DB;DP=10;NS=2\tGT:GQ:DP:HQ:FT\t1|2:21:6:23,27:SnpFilter\t2|1:2:0:18,2:SnpFilter\t2/2:35:4:.:SnpFilter\n"
        "20\t1230237\t.\tT\t.\t47\tPASS\tAA=T;DP=13;NS=3\tGT:GQ:DP:HQ:FT\t0|0:54:7:56,60:SnpFilter\t0|0:48:4:51,51:PASS\t0/0:.:2:.:.\n"
        "20\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tAA=G;DP=9;NS=3\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:3:3\n"
        "21\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tAA=G;DP=9;NS=3\t.\t.\t.\t.\n"
        "22\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\t.\t.\t.\t.\t.\n"
        ;

    string filteredTwiceLine =
        "20\t14370\trs6054257\tG\tA\t29\tq10;s50\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t1/1:43:5:.,.\n"
        ;
}

class TestVcfEntry : public ::testing::Test {
protected:
    void SetUp() {
        stringstream hdrss(headerText);
        InputStream in("test", hdrss);
        _header = Header::fromStream(in);

        stringstream vcfss(vcfLines);
        string line;
        while (getline(vcfss, line)) {
            Entry e(&_header, line);
            v.push_back(std::move(e));
        }
    }

    Entry makeEntry(string chrom, int64_t pos, string const& ref, string const& alt) {
        stringstream ss;
        ss << chrom << "\t" << pos << "\t.\t" << ref << "\t" << alt << "\t.\t.\t.";
        return Entry(&_header, ss.str());
    }

    Header _header;
    vector<Entry> v;
};

TEST_F(TestVcfEntry, default_compare) {
    EXPECT_LT(v[0], v[1]);
    EXPECT_FALSE(v[1] < v[0]);
}

TEST_F(TestVcfEntry, start_and_stop_without_padding) {
    auto const& entry = v[4];
    EXPECT_EQ(entry.startWithoutPadding(), 1234567);
    EXPECT_EQ(entry.stopWithoutPadding(), 1234570);
}

TEST_F(TestVcfEntry, parse) {
    stringstream ss(vcfLines);
    for (auto i = v.begin(); i != v.end(); ++i) {
        string line;
        getline(ss, line);
        ASSERT_EQ(line, i->toString());
    }

    ASSERT_EQ(7u, v.size());
    ASSERT_EQ("20", v[0].chrom());
    ASSERT_EQ(14370u, v[0].pos());
    ASSERT_EQ(14369, v[0].start());
    ASSERT_EQ(14370, v[0].stop());
    ASSERT_EQ("G", v[0].ref());
    ASSERT_EQ(1u, v[0].alt().size());
    ASSERT_EQ("A", v[0].alt()[0]);
    ASSERT_EQ(29.0, v[0].qual());
    ASSERT_TRUE(v[0].failedFilters().empty());
    ASSERT_EQ(5u, v[0].info().size());
    Entry::CustomValueMap info = v[0].info();
    ASSERT_EQ("3", info["NS"].getString(0));
    ASSERT_EQ("14", info["DP"].getString(0));
    ASSERT_EQ("0.5", info["AF"].getString(0));

    ASSERT_EQ(2u, v[2].alt().size());
    ASSERT_EQ("G", v[2].alt()[0]);
    ASSERT_EQ("T", v[2].alt()[1]);
}

TEST_F(TestVcfEntry, variantAdaptor) {
    vector<VariantAdaptor> va;
    for (auto i = v.begin(); i != v.end(); ++i)
        va.push_back(VariantAdaptor(*i));

    // 20 14370 rs6054257 G A 29 PASS NS=3;DP=14;AF=0.5;DB;H2 GT:GQ:DP:HQ 0|0:48:1:51,51 1|0:48:8:51,51 1/1:43:5:.,.
    ASSERT_EQ("20", va[0].chrom());
    ASSERT_EQ(14369, va[0].start());
    ASSERT_EQ(14370, va[0].stop());

    // 20 17330 . T A 3 q10 NS=3;DP=11;AF=0.017 GT:GQ:DP:HQ 0|0:49:3:58,50 0|1:3:5:65,3 0/0:41:3
    ASSERT_EQ("20", va[1].chrom());
    ASSERT_EQ(17329, va[1].start());
    ASSERT_EQ(17330, va[1].stop());

    // 20 1110696 rs6040355 A G,T 67 PASS NS=2;DP=10;AF=0.333,0.667;AA=T;DB GT:GQ:DP:HQ 1|2:21:6:23,27 2|1:2:0:18,2 2/2:35:4
    ASSERT_EQ("20", va[2].chrom());
    ASSERT_EQ(1110695, va[2].start());
    ASSERT_EQ(1110696, va[2].stop());

    // 20 1230237 . T . 47 PASS NS=3;DP=13;AA=T GT:GQ:DP:HQ 0|0:54:7:56,60 0|0:48:4:51,51 0/0:.:2"
    ASSERT_EQ("20", va[3].chrom());
    ASSERT_EQ(1230237, va[3].start());
    ASSERT_EQ(1230237, va[3].stop());

    // 20 1234567 microsat1 GTC G,GTCT 50 PASS NS=3;DP=9;AA=G GT:GQ:DP 0/1:35:4 0/2:17:2 1/1:40:3
    ASSERT_EQ("20", va[4].chrom());
    ASSERT_EQ(1234567, va[4].start());
    ASSERT_EQ(1234570, va[4].stop());
}

TEST_F(TestVcfEntry, badCustomTypes) {
    string badInfo = "20\t14370\t.\tG\tA\t29\tPASS\tQQ;\tGT:GQ:DP:HQ\n";
    string badFormat = "20\t14370\t.\tG\tA\t29\tPASS\t.;\tQQ\n";

    Entry e(&_header, badInfo);
    EXPECT_THROW(e.info(), std::runtime_error);

    e = Entry(&_header, badFormat);
    EXPECT_THROW(e.sampleData(), std::runtime_error);
}

TEST_F(TestVcfEntry, swap) {
    Entry e1 = v[0];
    Entry e2 = v[1];
    string str1 = e1.toString();
    string str2 = e2.toString();

    e1.swap(e2);
    ASSERT_EQ(str1, e2.toString());
    ASSERT_EQ(str2, e1.toString());

    // this was a bug
    ASSERT_EQ(&e1.header(), &v[1].header());
    ASSERT_EQ(&e2.header(), &v[0].header());

    ASSERT_EQ(e1.toString(), v[1].toString());
    ASSERT_EQ(e2.toString(), v[0].toString());
}

TEST_F(TestVcfEntry, samplesWithData) {
    Entry e(&_header,
        "22\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\t.\tGT\t.\t.\t0|1\n"
        );
    ASSERT_EQ(1u, e.sampleData().size());
    ASSERT_EQ(1u, e.sampleData().samplesWithData());

    ASSERT_EQ(3u, v[0].sampleData().samplesWithData());
    ASSERT_EQ(2u, v[1].sampleData().samplesWithData());
    ASSERT_EQ(3u, v[2].sampleData().samplesWithData());
    ASSERT_EQ(3u, v[3].sampleData().samplesWithData());
    ASSERT_EQ(3u, v[4].sampleData().samplesWithData());
    ASSERT_EQ(0u, v[5].sampleData().samplesWithData());
}

TEST_F(TestVcfEntry, samplesEvaluatedByFilter) {
    ASSERT_EQ(-1, v[0].sampleData().samplesEvaluatedByFilter());
    ASSERT_EQ(1, v[1].sampleData().samplesEvaluatedByFilter());
    ASSERT_EQ(3, v[2].sampleData().samplesEvaluatedByFilter());
    ASSERT_EQ(2, v[3].sampleData().samplesEvaluatedByFilter());
}

TEST_F(TestVcfEntry, samplesFailedFilter) {
    ASSERT_EQ(-1, v[0].sampleData().samplesFailedFilter());
    ASSERT_EQ(0, v[1].sampleData().samplesFailedFilter());
    ASSERT_EQ(3, v[2].sampleData().samplesFailedFilter());
    ASSERT_EQ(1, v[3].sampleData().samplesFailedFilter());
}

TEST_F(TestVcfEntry, removeLowDepthGenotypes) {
    ASSERT_EQ(3u, v[4].sampleData().samplesWithData());
    v[4].sampleData().removeLowDepthGenotypes(2);
    ASSERT_EQ(3u, v[4].sampleData().samplesWithData());
    v[4].sampleData().removeLowDepthGenotypes(4);
    ASSERT_EQ(1u, v[4].sampleData().samplesWithData());
}

TEST_F(TestVcfEntry, reheader) {
    stringstream ss(header2Text);
    Header merged = Header::fromStream(ss);
    Entry const& e = v[0];
    merged.merge(_header, true); // true to allow sample name overlaps
    SampleData origGT = e.sampleData();
    ASSERT_EQ(3u, origGT.size());
    ASSERT_EQ(&_header, &e.header());
    v[0].reheader(&merged);
    ASSERT_EQ(&merged, &e.header());
    ASSERT_EQ(3u, e.sampleData().size());
    ASSERT_EQ(4u, e.header().sampleCount());

    SampleData newGT = e.sampleData();
    ASSERT_EQ(0u, newGT.count(0));
    ASSERT_EQ(1u, newGT.count(1));
    ASSERT_EQ(1u, newGT.count(2));
    ASSERT_EQ(1u, newGT.count(3));
    ASSERT_TRUE(*origGT.get(2) == *newGT.get(1));
    ASSERT_TRUE(*origGT.get(1) == *newGT.get(2));
    ASSERT_TRUE(*origGT.get(0) == *newGT.get(3));
}

TEST_F(TestVcfEntry, genotypeForSample) {
    GenotypeCall gt;

    gt = v[0].sampleData().genotype(0);
    ASSERT_EQ(2u, gt.size());
    ASSERT_EQ(0u, gt[0]);
    ASSERT_EQ(0u, gt[1]);

    gt = v[0].sampleData().genotype(1);
    ASSERT_EQ(2u, gt.size());
    ASSERT_EQ(1u, gt[0]);
    ASSERT_EQ(0u, gt[1]);

    gt = v[0].sampleData().genotype(2);
    ASSERT_EQ(2u, gt.size());
    ASSERT_EQ(1u, gt[0]);
    ASSERT_EQ(1u, gt[1]);

    gt = v[1].sampleData().genotype(0);
    ASSERT_EQ(2u, gt.size());
    ASSERT_EQ(0u, gt[0]);
    ASSERT_EQ(0u, gt[1]);

    gt = v[1].sampleData().genotype(1);
    ASSERT_EQ(2u, gt.size());
    ASSERT_EQ(0u, gt[0]);
    ASSERT_EQ(1u, gt[1]);

    gt = v[1].sampleData().genotype(2);
    ASSERT_TRUE(gt.empty());
}

TEST_F(TestVcfEntry, addFilter) {
    Entry e1 = v[0];

    ASSERT_THROW(e1.addFilter(string("sq;50")),runtime_error);
    ASSERT_THROW(e1.addFilter(string("sq 50")),runtime_error);
    ASSERT_NO_THROW(e1.addFilter(string("sq.50")));
    ASSERT_NO_THROW(e1.addFilter(string("sq50")));
    ASSERT_FALSE(e1.toString() == v[0].toString());
}

// this is an efficiency test
// it makes sure that move semantics are working properly (data is stolen from
// rvalue references in the move constructor)
TEST_F(TestVcfEntry, move) {
    Entry& e(v[0]);
    // we need to have data in every field for this test, so let's add a filter
    e.addFilter("s50");

    char const* addrChrom = e.chrom().data();
    char const* addrRef = e.ref().data();
    string const* addrAlt = e.alt().data();
    string const* addrFilter = &*e.failedFilters().begin();
    Entry::CustomValueMap::value_type const* addrInfo = &*e.info().begin();

    // nested sample data addresses
    SampleData const& sd = e.sampleData();
    CustomType const* const* addrFormat = sd.format().data();
    SampleData::MapType::value_type const* addrSampleValues = &*sd.begin();

    // primitive types are copied, not moved. we still check that they get
    // set properly though.
    uint64_t origPos = e.pos();
    int64_t origStart = e.start();
    int64_t origStop = e.stop();
    double origQual = e.qual();

    // do the move and check that the resulting output strings are the same
    // first
    stringstream orig;
    orig << e;
    Entry e2(std::move(e));
    stringstream moved;
    moved << e2;
    ASSERT_EQ(orig.str(), moved.str());

    // now check that the addresses of data members of e2 have been stolen
    // from the original e
    ASSERT_EQ(addrChrom, e2.chrom().data());
    ASSERT_EQ(origPos, e2.pos());
    ASSERT_EQ(addrRef, e2.ref().data());
    ASSERT_EQ(addrAlt, e2.alt().data());
    ASSERT_EQ(origQual, e2.qual());
    ASSERT_EQ(addrFilter, &*e2.failedFilters().begin());
    ASSERT_EQ(addrInfo, &*e2.info().begin());
    ASSERT_EQ(addrFormat, e2.sampleData().format().data());
    ASSERT_EQ(addrSampleValues, &*e2.sampleData().begin());
    ASSERT_EQ(origStart, e2.start());
    ASSERT_EQ(origStop, e2.stop());
}

TEST_F(TestVcfEntry, moreSamplesThanHeader) {
    Entry e(&_header, "20\t14370\tid1\tT\tC\t29\tPASS\t.\tGT\t0/1\t0/1\t0/1\t0/1");
    ASSERT_THROW(e.sampleData(), runtime_error);
    // we're allowing trailing tabs
    e = Entry(&_header, "20\t14370\tid1\tT\tC\t29\tPASS\t.\tGT\t0/1\t0/1\t0/1\t");
    ASSERT_NO_THROW(e.sampleData());
    ASSERT_EQ(3u, e.sampleData().samplesWithData());
}

TEST_F(TestVcfEntry, copy) {
    Entry e1(v[0]);
    string line = e1.toString();
    Vcf::Entry::parseLine(&_header, line, e1);
    ASSERT_EQ(e1.toString(), line);
}

TEST_F(TestVcfEntry, isFiltered) {
    EXPECT_TRUE(v[0].failedFilters().empty());
    EXPECT_FALSE(v[0].isFiltered());

    ASSERT_EQ(1u, v[1].failedFilters().size());
    EXPECT_EQ("q10", *v[1].failedFilters().begin());
    EXPECT_TRUE(v[1].isFiltered());

    ASSERT_EQ(1u, v[2].failedFilters().size());
    EXPECT_EQ("PASS", *v[2].failedFilters().begin());

    EXPECT_FALSE(v[2].isFiltered());
}

TEST_F(TestVcfEntry, allButSamplesToStream) {
    std::stringstream ss;
    std::string expected(
        "20\t14370\trs6054257\tG\tA\t29\t.\tAF=0.5;DB;DP=14;H2;NS=3"
        );

    v[0].allButSamplesToStream(ss);
    EXPECT_EQ(expected, ss.str());
}

TEST_F(TestVcfEntry, sampleDataPrintCertainSample) {
    std::vector<std::string> expected = list_of
        ("0|0:48:1:51,51")
        ("1|0:48:8:51,51")
        ("1/1:43:5:.,.")
        ;

    for (size_t i = 0; i < 3; ++i) {
        std::stringstream ss;
        v[0].sampleData().sampleToStream(ss, i);
        EXPECT_EQ(expected[i], ss.str());
    }
}

TEST_F(TestVcfEntry, multipleFilters) {
    stringstream vcfss(filteredTwiceLine);
    string line;
    ASSERT_TRUE(bool(getline(vcfss, line)));
    Entry e(&_header, line);

    EXPECT_EQ(2u, e.failedFilters().size());
    EXPECT_TRUE(e.isFiltered());
}

TEST_F(TestVcfEntry, multipleFiltersWhitelist) {
    stringstream vcfss(filteredTwiceLine);
    string line;
    ASSERT_TRUE(bool(getline(vcfss, line)));
    Entry e(&_header, line);

    EXPECT_EQ(2u, e.failedFilters().size());
    EXPECT_TRUE(e.isFiltered());

    std::set<std::string> whitelist;

    whitelist.insert("s50");
    EXPECT_TRUE(e.isFiltered());

    whitelist.clear();
    whitelist.insert("q10");
    EXPECT_TRUE(e.isFiltered());

    whitelist.insert("s50");
    whitelist.insert("q10");
    EXPECT_FALSE(e.isFilteredByAnythingExcept(whitelist));
}
