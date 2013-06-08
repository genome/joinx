#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/InferFileType.hpp"

#include <gtest/gtest.h>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace std::placeholders;
using namespace Vcf;

namespace {
    string testData =
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
        "20\t14370\trs6054257\tG\tA\t29\tPASS\tNS=3;DP=14;AF=0.5;DB;H2\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t1/1:43:5:.,.\n"
        "20\t17330\t.\tT\tA\t3\tq10\tNS=3;DP=11;AF=0.017\tGT:GQ:DP:HQ\t0|0:49:3:58,50\t0|1:3:5:65,3\t0/0:41:3\n"
        "20\t1110696\trs6040355\tA\tG,T\t67\tPASS\tNS=2;DP=10;AF=0.333,0.667;AA=T;DB\tGT:GQ:DP:HQ\t1|2:21:6:23,27\t2|1:2:0:18,2\t2/2:35:4\n"
        "20\t1230237\t.\tT\t.\t47\tPASS\tNS=3;DP=13;AA=T\tGT:GQ:DP:HQ\t0|0:54:7:56,60\t0|0:48:4:51,51\t0/0:61:2\n"
        "20\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3\n"
        ;
}

TEST(VcfReader, read) {
    stringstream ss(testData);
    InputStream in("test", ss);
    inferFileType(in);

    typedef function<void(const Vcf::Header*, string&, Entry&)> Extractor;
    typedef TypedStream<Entry, Extractor> VcfReader;
    Extractor extractor = bind(&Entry::parseLine, _1, _2, _3);
    VcfReader r(extractor, in);
    Entry e;

    const Vcf::Header& h = r.header();

    ASSERT_EQ(19u, r.lineNum()); //This prolly shouldn't be here as it is a test for TypedStream not VcfReader
    ASSERT_EQ(6u, h.infoTypes().size());
    ASSERT_EQ(2u, h.filters().size());
    ASSERT_EQ(4u, h.formatTypes().size());

    uint64_t lineCount = 19;
    vector<Entry> v;
    while (r.next(e)) {
        v.push_back(e);

        ASSERT_EQ(++lineCount,r.lineNum());   //should be at line 1 of the VCF lines (it skips the header)
    }

    ASSERT_EQ(5u, v.size());
    ASSERT_EQ(14370u, v[0].pos());
    ASSERT_EQ(17330u, v[1].pos());
    ASSERT_EQ(1110696u, v[2].pos());
    ASSERT_EQ(1230237u, v[3].pos());
    ASSERT_EQ(1234567u, v[4].pos());
}
