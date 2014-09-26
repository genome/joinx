#include "fileformats/vcf/CustomType.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace Vcf;

namespace {
    std::vector<std::string> scalars{
          "ID=NS,Number=1,Type=Integer,Description=\"desc\""
        , "ID=NS,Number=1,Type=String,Description=\"desc\""
        , "ID=NS,Number=1,Type=Float,Description=\"desc\""
        , "ID=NS,Number=1,Type=Character,Description=\"desc\""
        , "ID=NS,Number=1,Type=Flag,Description=\"desc\""
        };

    std::vector<std::string> nonScalars{
          "ID=NS,Number=.,Type=Integer,Description=\"desc\""
        , "ID=NS,Number=A,Type=String,Description=\"desc\""
        , "ID=NS,Number=G,Type=Float,Description=\"desc\""
        , "ID=NS,Number=2,Type=Character,Description=\"desc\""
        , "ID=NS,Number=3,Type=Flag,Description=\"desc\""
        };
}

TEST(TestVcfCustomType, isScalar) {
    for (auto i = scalars.begin(); i != scalars.end(); ++i) {
        CustomType t(*i);
        EXPECT_TRUE(t.isScalar()) << *i << " should evaluate to a scalar type";
    }

    for (auto i = nonScalars.begin(); i != nonScalars.end(); ++i) {
        CustomType t(*i);
        EXPECT_FALSE(t.isScalar()) << *i << " should not evaluate to a scalar type";
    }
}

TEST(TestVcfCustomType, fromString) {
    string s("ID=NS,Number=1,Type=Integer,Description=\"desc1=cool\"");
    CustomType t1(s);
    ASSERT_EQ("NS", t1.id());
    ASSERT_EQ(CustomType::FIXED_SIZE, t1.numberType());
    ASSERT_EQ(1u, t1.number());
    ASSERT_EQ("desc1=cool", t1.description());
    ASSERT_EQ(s, t1.toString());
}

TEST(TestVcfCustomType, negativeNumber) {
    string s("ID=NS,Number=-1,Type=Integer,Description=\"desc1=cool\"");
    CustomType t1(s);
    ASSERT_EQ("NS", t1.id());
    ASSERT_EQ(CustomType::VARIABLE_SIZE, t1.numberType());
    ASSERT_EQ(0u, t1.number());
    ASSERT_EQ("desc1=cool", t1.description());
    ASSERT_EQ("ID=NS,Number=.,Type=Integer,Description=\"desc1=cool\"", t1.toString());
}
