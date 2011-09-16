#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/VariantAdaptor.hpp"
#include "fileformats/InputStream.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;

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
        "20\t14370\trs6054257\tG\tA\t29\tPASS\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t1/1:43:5:.,.\n"
        "20\t17330\t.\tT\tA\t3\tq10\tAF=0.017;DP=11;NS=3\tGT:GQ:DP:HQ\t0|0:49:3:58,50\t0|1:3:5:65,3\t0/0:41:3\n"
        "20\t1110696\trs6040355\tA\tG,T\t67\tPASS\tAA=T;AF=0.333,0.667;DB;DP=10;NS=2\tGT:GQ:DP:HQ\t1|2:21:6:23,27\t2|1:2:0:18,2\t2/2:35:4\n"
        "20\t1230237\t.\tT\t.\t47\tPASS\tAA=T;DP=13;NS=3\tGT:GQ:DP:HQ\t0|0:54:7:56,60\t0|0:48:4:51,51\t0/0:61:2\n"
        "20\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tAA=G;DP=9;NS=3\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3\n"
        ;
}

class TestVcfEntry : public ::testing::Test {
protected:
    void SetUp() {
        stringstream ss(headerText);
        InputStream in("test", ss);
        _header = Header::fromStream(in);
    }

    Header _header;
};

TEST_F(TestVcfEntry, parse) {
    stringstream ss(vcfLines);
    string line;
    vector<Entry> v;
    while (getline(ss, line)) {
        Entry e(&_header, line);
        stringstream ss;
        ss << e;
        ASSERT_EQ(line, ss.str());
        v.push_back(e);
    }

    ASSERT_EQ(5, v.size());
    ASSERT_EQ("20", v[0].chrom());
    ASSERT_EQ(14370, v[0].pos());
    ASSERT_EQ("G", v[0].ref());
    ASSERT_EQ(1, v[0].alt().size());
    ASSERT_EQ("A", v[0].alt()[0]);
    ASSERT_EQ(29.0, v[0].qual());
    ASSERT_TRUE(v[0].failedFilters().empty());
    ASSERT_EQ(5, v[0].info().size());
    Entry::CustomValueMap info = v[0].info();
    ASSERT_EQ("3", info["NS"].getString(0));
    ASSERT_EQ("14", info["DP"].getString(0));
    ASSERT_EQ("0.5", info["AF"].getString(0));
//    ASSERT_EQ(InfoField("DB"), v[0].info()[3]);
//    ASSERT_EQ(InfoField("H2"), v[0].info()[4]);

    ASSERT_EQ(2, v[2].alt().size());
    ASSERT_EQ("G", v[2].alt()[0]);
    ASSERT_EQ("T", v[2].alt()[1]);
}

TEST_F(TestVcfEntry, variantAdaptor) {
    stringstream ss(vcfLines);
    string line;
    vector<VariantAdaptor> v;
    while (getline(ss, line)) {
        Entry e(&_header, line);
        stringstream ss;
        ss << e;
        ASSERT_EQ(line, ss.str());
        v.push_back(VariantAdaptor(e));
    }

    // 20 14370 rs6054257 G A 29 PASS NS=3;DP=14;AF=0.5;DB;H2 GT:GQ:DP:HQ 0|0:48:1:51,51 1|0:48:8:51,51 1/1:43:5:.,.
    ASSERT_EQ("20", v[0].chrom());
    ASSERT_EQ(14369, v[0].start());
    ASSERT_EQ(14370, v[0].stop());

    // 20 17330 . T A 3 q10 NS=3;DP=11;AF=0.017 GT:GQ:DP:HQ 0|0:49:3:58,50 0|1:3:5:65,3 0/0:41:3
    ASSERT_EQ("20", v[1].chrom());
    ASSERT_EQ(17329, v[1].start());
    ASSERT_EQ(17330, v[1].stop());

    // 20 1110696 rs6040355 A G,T 67 PASS NS=2;DP=10;AF=0.333,0.667;AA=T;DB GT:GQ:DP:HQ 1|2:21:6:23,27 2|1:2:0:18,2 2/2:35:4
    ASSERT_EQ("20", v[2].chrom());
    ASSERT_EQ(1110695, v[2].start());
    ASSERT_EQ(1110696, v[2].stop());

    // 20 1230237 . T . 47 PASS NS=3;DP=13;AA=T GT:GQ:DP:HQ 0|0:54:7:56,60 0|0:48:4:51,51 0/0:61:2"
    ASSERT_EQ("20", v[3].chrom());
    ASSERT_EQ(1230237, v[3].start());
    ASSERT_EQ(1230237, v[3].stop());

    // 20 1234567 microsat1 GTC G,GTCT 50 PASS NS=3;DP=9;AA=G GT:GQ:DP 0/1:35:4 0/2:17:2 1/1:40:3
    ASSERT_EQ("20", v[4].chrom());
    ASSERT_EQ(1234567, v[4].start());
    ASSERT_EQ(1234570, v[4].stop());
}

TEST_F(TestVcfEntry, badCustomTypes) {
    Entry e;
    string badInfo = "20\t14370\t.\tG\tA\t29\tPASS\tQQ;\tGT:GQ:DP:HQ\n";
    string badFormat = "20\t14370\t.\tG\tA\t29\tPASS\t.;\tQQ\n";
    ASSERT_THROW(Entry(&_header, badInfo), runtime_error);
    ASSERT_THROW(Entry(&_header, badFormat), runtime_error);
}
