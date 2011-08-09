#include "fileformats/Vcf.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;


TEST(Vcf, strToType) {
    ASSERT_EQ(INT, strToType("Integer"));
    ASSERT_EQ(FLOAT, strToType("Float"));
    ASSERT_EQ(CHAR, strToType("Character"));
    ASSERT_EQ(STRING, strToType("String"));
    ASSERT_EQ(FLAG, strToType("Flag"));
    ASSERT_THROW(strToType("something"), runtime_error);
}

TEST(Vcf, parseString) {
    ASSERT_THROW(parseString("foo"), runtime_error);
    ASSERT_THROW(parseString("\"foo"), runtime_error);
    ASSERT_THROW(parseString("foo\""), runtime_error);
    ASSERT_THROW(parseString("foo\\"), runtime_error);

    ASSERT_EQ("foo", parseString("\"foo\""));
    ASSERT_EQ("foo\\", parseString("\"foo\\\\\""));
    ASSERT_EQ("f\"o\\o\"", parseString("\"f\\\"o\\\\o\\\"\""));
}

TEST(Vcf, propLine) {
    vector<string> required = { "ID", "Number", "Type", "Description" };
    Header::PropLine line("ID=AF,Number=A,Type=Float,Description=\"Allele Frequency\"", required);
    ASSERT_EQ("AF", line["ID"]);
    ASSERT_EQ("A", line["Number"]);
    ASSERT_EQ("Float", line["Type"]);
    ASSERT_EQ("\"Allele Frequency\"", line["Description"]);
}

TEST(Vcf, parse) {
    string lines =
        "20\t14370\trs6054257\tG\tA\t29\tPASS\tNS=3;DP=14;AF=0.5;DB;H2\tGT:GQ:DP:HQ\t0|0:48:1:51,51\t1|0:48:8:51,51\t1/1:43:5:.,.\n"
        "20\t17330\t.\tT\tA\t3\tq10\tNS=3;DP=11;AF=0.017\tGT:GQ:DP:HQ\t0|0:49:3:58,50\t0|1:3:5:65,3\t0/0:41:3\n"
        "20\t1110696\trs6040355\tA\tG,T\t67\tPASS\tNS=2;DP=10;AF=0.333,0.667;AA=T;DB\tGT:GQ:DP:HQ\t1|2:21:6:23,27\t2|1:2:0:18,2\t2/2:35:4\n"
        "20\t1230237\t.\tT\t.\t47\tPASS\tNS=3;DP=13;AA=T\tGT:GQ:DP:HQ\t0|0:54:7:56,60\t0|0:48:4:51,51\t0/0:61:2\n"
        "20\t1234567\tmicrosat1\tGTC\tG,GTCT\t50\tPASS\tNS=3;DP=9;AA=G\tGT:GQ:DP\t0/1:35:4\t0/2:17:2\t1/1:40:3\n";

    stringstream ss(lines);
    string line;
    while (getline(ss, line)) {
        Entry e(line);
        stringstream ss;
        ss << e;
        ASSERT_EQ(line, ss.str());
    }
}
