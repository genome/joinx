#include "common/Variant.hpp"
#include "fileformats/Bed.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;

namespace {
    Bed makeBedQualDepth(const std::string& qual, const std::string& depth) {
        vector<string> extra;
        extra.push_back("A/T");
        if (!qual.empty()) extra.push_back(qual);
        if (!depth.empty()) extra.push_back(depth);
        return Bed("1", 1, 2, extra);
    }
}

TEST(Variant, parseQualityAndDepth) {
    Bed b1 = makeBedQualDepth("3", "4");
    Variant v1(b1);
    ASSERT_EQ(3, v1.quality());
    ASSERT_EQ(4, v1.depth());

    Bed b2 = makeBedQualDepth("CAT", "");
    ASSERT_THROW(Variant v2(b2), runtime_error);

    Bed b3 = makeBedQualDepth("5", "PIG");
    ASSERT_THROW(Variant v3(b3), runtime_error);

}
