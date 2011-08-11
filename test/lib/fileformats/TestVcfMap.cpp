#include "fileformats/vcf/Map.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;

TEST(VcfMap, parse) {
    string s = "ID=NS,Number=1,Type=Integer,Description=\"foo\"";
    Map m(s); 
    ASSERT_EQ(4, m.size());
    ASSERT_EQ("NS", m["ID"]);
    ASSERT_EQ("1", m["Number"]);
    ASSERT_EQ("Integer", m["Type"]);
    ASSERT_EQ("\"foo\"", m["Description"]);
}

TEST(VcfMap, duplicateKeyError) {
    string s = "ID=NS,ID=no,Number=1,Type=Integer,Description=\"foo\"";
    ASSERT_THROW(Map m(s), runtime_error);

    Map m;
    m.insert("ID", "foo");
    ASSERT_THROW(m.insert("ID", "foo"), runtime_error);
}

TEST(VcfMap, nonExistingKey) {
    Map m;
    ASSERT_THROW(m["foo"], runtime_error);
}

TEST(VcfMap, toString) {
    Map m;
    m.insert("ID", "foo");
    m.insert("dbsnp", "");
    m.insert("cheeseburger", "good");
    ASSERT_EQ("ID=foo,dbsnp,cheeseburger=good", m.toString());
}
