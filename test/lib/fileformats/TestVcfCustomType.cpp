#include "fileformats/vcf/CustomType.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;

TEST(VcfCustomValue, fromString) {
    string s("ID=NS,Number=1,Type=Integer,Description=\"desc1=cool\"");
    CustomType t1(s);
    ASSERT_EQ("NS", t1.id());
    ASSERT_EQ(CustomType::FIXED_SIZE, t1.numberType());
    ASSERT_EQ(1, t1.number());
    ASSERT_EQ("desc1=cool", t1.description());
    ASSERT_EQ(s, t1.toString());
}

TEST(VcfCustomValue, negativeNumber) {
    string s("ID=NS,Number=-1,Type=Integer,Description=\"desc1=cool\"");
    CustomType t1(s);
    ASSERT_EQ("NS", t1.id());
    ASSERT_EQ(CustomType::VARIABLE_SIZE, t1.numberType());
    ASSERT_EQ(0, t1.number());
    ASSERT_EQ("desc1=cool", t1.description());
    ASSERT_EQ("ID=NS,Number=.,Type=Integer,Description=\"desc1=cool\"", t1.toString());
}
