#include "fileformats/Variant.hpp"
#include "fileformats/Bed.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <gtest/gtest.h>

using namespace std;

namespace {
    Bed mkBed(const std::string& refCall, const std::string& qual, const std::string& depth) {
        vector<string> extra;
        extra.push_back(refCall);
        if (!qual.empty()) extra.push_back(qual);
        if (!depth.empty()) extra.push_back(depth);
        return Bed("1", 1, 2, extra);
    }
}

TEST(Variant, parseQualityAndDepth) {
    Bed b1 = mkBed("A/T", "3", "4");
    Variant v1(b1);
    ASSERT_EQ(3, v1.quality());
    ASSERT_EQ(4, v1.depth());

    Bed b2 = mkBed("A/T", "CAT", "");
    ASSERT_THROW(Variant v2(b2), runtime_error);

    Bed b3 = mkBed("A/T", "5", "PIG");
    ASSERT_THROW(Variant v3(b3), runtime_error);

}

TEST(Variant, partialAlleleMatch) {
    map<string, string> valid;
    valid["A"] = "AMRWDHVN";
    valid["C"] = "CMYSBHVN";
    valid["G"] = "GKRSDBVN";
    valid["T"] = "TKYWDBHN";

    map<string, string> invalid;
    invalid["A"] = "CGTKYSB";
    invalid["C"] = "AGTKRWD";
    invalid["G"] = "ACTMYWH";
    invalid["T"] = "ACGMRSV";
    
    typedef map<string,string>::const_iterator IterType;
    for (IterType iter = valid.begin(); iter != valid.end(); ++iter) {
        stringstream strA;
        strA << iter->first << "/" << iter->first;
        Variant a(mkBed(strA.str(), "0", "0"));
        for (unsigned i = 0; i < iter->second.size(); ++i) {
            stringstream strB;
            strB << iter->first << "/" << iter->second[i];
            Variant b(mkBed(strB.str(), "0", "0"));
            ASSERT_TRUE(a.allelePartialMatch(b));
            ASSERT_TRUE(b.allelePartialMatch(a));
        }
    }

    for (IterType iter = invalid.begin(); iter != invalid.end(); ++iter) {
        stringstream strA;
        strA << iter->first << "/" << iter->first;
        Variant a(mkBed(strA.str(), "0", "0"));
        for (unsigned i = 0; i < iter->second.size(); ++i) {
            stringstream strB;
            strB << iter->first << "/" << iter->second[i];
            Variant b(mkBed(strB.str(), "0", "0"));
            ASSERT_FALSE(a.allelePartialMatch(b)) 
                << strA.str() << " vs " << strB.str() << " expected no match";
            ASSERT_FALSE(b.allelePartialMatch(a));
        }
    }
}

TEST(Variant, floatScore) {
    Variant v(mkBed("1", "3.456", "7"));
    ASSERT_EQ("1", v.chrom());
    ASSERT_NEAR(double(3.456), v.quality(), 1e-7);
    ASSERT_EQ(7, v.depth());
}

TEST(Variant, dashQualDepth) {
    Variant v(mkBed("1", "-", "-"));
    ASSERT_EQ(0.0, v.depth());
    ASSERT_EQ(0.0, v.quality());
}
