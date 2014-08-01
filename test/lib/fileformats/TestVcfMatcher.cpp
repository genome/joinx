#include "fileformats/vcf/Matcher.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/VariantAdaptor.hpp"
#include "io/InputStream.hpp"

#include <boost/bind.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace Vcf;
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
        "20\t14370\trs6054257\tG\tA\t29\tPASS\tAF=0.5;DB;DP=14;H2;NS=3\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t1/1:43:5:.,.\n"
        "20\t17330\t.\tT\tA\t3\tq10\tAF=0.017;DP=11;NS=3\tGT:GQ:DP:HQ\t0|0:49:3:58,50\t0|1:3:5:65,3\t.\n"
        "20\t1110696\trs6040355\tA\tG,T\t67\tPASS\tAA=T;AF=0.333,0.667;DB;DP=10;NS=2\tGT:GQ:DP:HQ\t1|2:21:6:23,27\t2|1:2:0:18,2\t2/2:35:4\n"
        "20\t1230237\t.\tT\t.\t47\tPASS\tAA=T;DP=13;NS=3\tGT:GQ:DP:HQ\t0|0:54:7:56,60\t0|0:48:4:51,51\t0/0:.:2\n"
        "20\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tAA=G;DP=9;NS=3\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:3:3\n"
        "21\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tAA=G;DP=9;NS=3\t.\n"
        ;
}

class TestVcfMatcher : public ::testing::Test {
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

TEST_F(TestVcfMatcher, matchChromLT) {
    typedef Conditions::RelOp<string, less<string> > LT;
    string val("21");
    LT::Extractor ext = boost::bind(&Entry::chrom, _1);
    LT chromMatch(val, ext);

    ASSERT_TRUE(chromMatch(v[0]));
    ASSERT_TRUE(chromMatch(v[1]));
    ASSERT_TRUE(chromMatch(v[2]));
    ASSERT_TRUE(chromMatch(v[3]));
    ASSERT_TRUE(chromMatch(v[4]));
    ASSERT_FALSE(chromMatch(v[5]));
}

TEST_F(TestVcfMatcher, matchPos) {
    typedef Conditions::RelOp<uint64_t, less<uint64_t> > LT;
    LT::Extractor ext = boost::bind(&Entry::pos, _1);
    uint64_t val = 1200000;
    LT posMatch(val, ext);

    ASSERT_TRUE(posMatch(v[0]));
    ASSERT_TRUE(posMatch(v[1]));
    ASSERT_TRUE(posMatch(v[2]));
    ASSERT_FALSE(posMatch(v[3]));
    ASSERT_FALSE(posMatch(v[4]));
    ASSERT_FALSE(posMatch(v[5]));
}

TEST_F(TestVcfMatcher, posNotLess) {
    typedef Conditions::RelOp<uint64_t, less<uint64_t> > LT;
    LT::Extractor ext = boost::bind(&Entry::pos, _1);
    uint64_t val = 1200000;
    LT posMatch(val, ext);
    Conditions::Not n(posMatch);

    ASSERT_FALSE(n(v[0]));
    ASSERT_FALSE(n(v[1]));
    ASSERT_FALSE(n(v[2]));
    ASSERT_TRUE(n(v[3]));
    ASSERT_TRUE(n(v[4]));
    ASSERT_TRUE(n(v[5]));
}

TEST_F(TestVcfMatcher, refEq) {
    typedef Conditions::RelOp<string, equal_to<string> > EQ;
    EQ::Extractor ext = boost::bind(&Entry::ref, _1);
    string val("G");
    EQ refMatch(val, ext);

    ASSERT_TRUE(refMatch(v[0]));
    ASSERT_FALSE(refMatch(v[1]));
    ASSERT_FALSE(refMatch(v[2]));
    ASSERT_FALSE(refMatch(v[3]));
    ASSERT_FALSE(refMatch(v[4]));
    ASSERT_FALSE(refMatch(v[5]));
}

TEST_F(TestVcfMatcher, logicalAnd) {
    typedef Conditions::RelOp<string, equal_to<string> > StrEq;
    typedef Conditions::RelOp<uint64_t, greater<uint64_t> > IntGt;
    typedef Conditions::BinaryLogical< logical_and<bool> > And;

    StrEq::Extractor ext1 = boost::bind(&Entry::chrom, _1);
    string val1("20");
    StrEq chromMatch(val1, ext1);

    IntGt::Extractor ext2 = boost::bind(&Entry::pos, _1);
    uint64_t val2 = 1200000;
    IntGt posMatch(val2, ext2);

    And and1(&posMatch, &chromMatch);
    And and2(&chromMatch, &posMatch);

    ASSERT_FALSE(and1(v[0]));
    ASSERT_FALSE(and1(v[1]));
    ASSERT_FALSE(and1(v[2]));
    ASSERT_TRUE(and1(v[3]));
    ASSERT_TRUE(and1(v[4]));
    ASSERT_FALSE(and1(v[5]));

    ASSERT_FALSE(and2(v[0]));
    ASSERT_FALSE(and2(v[1]));
    ASSERT_FALSE(and2(v[2]));
    ASSERT_TRUE(and2(v[3]));
    ASSERT_TRUE(and2(v[4]));
    ASSERT_FALSE(and2(v[5]));
}
