#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/VariantAdaptor.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;

namespace {
    string vcfLines =
        "20\t14370\trs6054257\tG\tA\t29\tPASS\tNS=3;DP=14;AF=0.5;DB;H2\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t1/1:43:5:.,.\n"
        "20\t17330\t.\tT\tA\t3\tq10\tNS=3;DP=11;AF=0.017\tGT:GQ:DP:HQ\t0|0:49:3:58,50\t0|1:3:5:65,3\t0/0:41:3\n"
        "20\t1110696\trs6040355\tA\tG,T\t67\tPASS\tNS=2;DP=10;AF=0.333,0.667;AA=T;DB\tGT:GQ:DP:HQ\t1|2:21:6:23,27\t2|1:2:0:18,2\t2/2:35:4\n"
        "20\t1230237\t.\tT\t.\t47\tPASS\tNS=3;DP=13;AA=T\tGT:GQ:DP:HQ\t0|0:54:7:56,60\t0|0:48:4:51,51\t0/0:61:2\n"
        "20\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3\n"
        ;
}

TEST(VcfEntry, parse) {
    stringstream ss(vcfLines);
    string line;
    vector<Entry> v;
    while (getline(ss, line)) {
        Entry e(line);
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
    ASSERT_EQ(InfoField("NS=3"), v[0].info()[0]);
    ASSERT_EQ(InfoField("DP=14"), v[0].info()[1]);
    ASSERT_EQ(InfoField("AF=0.5"), v[0].info()[2]);
    ASSERT_EQ(InfoField("DB"), v[0].info()[3]);
    ASSERT_EQ(InfoField("H2"), v[0].info()[4]);

    ASSERT_EQ(2, v[2].alt().size());
    ASSERT_EQ("G", v[2].alt()[0]);
    ASSERT_EQ("T", v[2].alt()[1]);
}

TEST(VcfEntry, variantAdaptor) {
    stringstream ss(vcfLines);
    string line;
    vector<VariantAdaptor> v;
    while (getline(ss, line)) {
        Entry e(line);
        stringstream ss;
        ss << e;
        ASSERT_EQ(line, ss.str());
        v.push_back(VariantAdaptor(e));
    }

    ASSERT_EQ("20", v[0].chrom());
    ASSERT_EQ(14369, v[0].start());
    ASSERT_EQ(14370, v[0].stop());
    ASSERT_EQ("G", v[0].reference());
    ASSERT_EQ("A", v[0].variant());
    ASSERT_FALSE(v[0].advance());

    ASSERT_EQ("20", v[1].chrom());
    ASSERT_EQ(17329, v[1].start());
    ASSERT_EQ(17330, v[1].stop());
    ASSERT_EQ("T", v[1].reference());
    ASSERT_EQ("A", v[1].variant());
    ASSERT_FALSE(v[1].advance());

    ASSERT_EQ("20", v[2].chrom());
    ASSERT_EQ(1110695, v[2].start());
    ASSERT_EQ(1110696, v[2].stop());
    ASSERT_EQ("A", v[2].reference());
    ASSERT_EQ("G", v[2].variant());
    ASSERT_TRUE(v[2].advance());
    ASSERT_EQ("20", v[2].chrom());
    ASSERT_EQ(1110695, v[2].start());
    ASSERT_EQ(1110696, v[2].stop());
    ASSERT_EQ("A", v[2].reference());
    ASSERT_EQ("T", v[2].variant());
    ASSERT_FALSE(v[2].advance());


    ASSERT_EQ("20", v[4].chrom());
    ASSERT_EQ(1234567, v[4].start());
    ASSERT_EQ(1234569, v[4].stop());
    ASSERT_EQ("TC", v[4].reference());
    ASSERT_EQ("", v[4].variant());
    ASSERT_TRUE(v[4].advance());
    ASSERT_EQ("20", v[4].chrom());
    ASSERT_EQ(1234570, v[4].start());
    ASSERT_EQ(1234570, v[4].stop());
    ASSERT_EQ("", v[4].reference());
    ASSERT_EQ("T", v[4].variant());
    ASSERT_FALSE(v[4].advance());
}
