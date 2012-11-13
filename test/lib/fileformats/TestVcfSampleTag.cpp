#include "fileformats/vcf/SampleTag.hpp"

#include <sstream>
#include <stdexcept>
#include <gtest/gtest.h>

using namespace Vcf;
using namespace std;

TEST(TestVcfSampleTag, parse) {
    {
        SampleTag st("ID=1,x=y");
        ASSERT_EQ("##SAMPLE=<ID=1,x=y>", st.toString());
        ASSERT_TRUE(st.get("x") != 0);
        ASSERT_TRUE(st.get("y") == 0);
        ASSERT_EQ("y", *st.get("x"));
        ASSERT_EQ("1", st.id());
    }

    {
        SampleTag st("ID=2,x=<1,2,3>");
        ASSERT_EQ("##SAMPLE=<ID=2,x=<1,2,3>>", st.toString());
        ASSERT_TRUE(st.get("x") != 0);
        ASSERT_EQ("<1,2,3>", *st.get("x"));
    }

    {
        SampleTag st("ID=3,x=\"y\"");
        ASSERT_EQ("##SAMPLE=<ID=3,x=\"y\">", st.toString());
        ASSERT_TRUE(st.get("x") != 0);
        ASSERT_EQ("\"y\"", *st.get("x"));
    }

    {
        SampleTag st("ID=4,x=<1,\"twenty point one\",four>");
        ASSERT_EQ("##SAMPLE=<ID=4,x=<1,\"twenty point one\",four>>",
            st.toString());
        ASSERT_TRUE(st.get("x") != 0);
        ASSERT_EQ("<1,\"twenty point one\",four>", *st.get("x"));

    }

    {
        SampleTag st("ID=5,flag,x=2");
        ASSERT_EQ("##SAMPLE=<ID=5,flag,x=2>", st.toString());
        ASSERT_TRUE(st.get("x") != 0);
        ASSERT_TRUE(*st.get("x") == "2");
        ASSERT_TRUE(st.get("flag") != 0);
        ASSERT_TRUE(st.get("flag")->empty());
    }
}
