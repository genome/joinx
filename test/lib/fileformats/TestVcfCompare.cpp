#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Compare.hpp"
#include "fileformats/InputStream.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace Vcf;
using namespace std::placeholders;
using namespace std;

namespace {
    string headerText(
        "##fileformat=VCFv4.1\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA00001\tNA00002\tNA00003\n"
        );

    template<typename MapType>
    bool mapHasPair(
            MapType const& m,
            typename MapType::key_type const& key,
            typename MapType::mapped_type const& val
            )
    {
        auto iter = m.find(key);
        if (iter == m.end())
            return false;
        return iter->second == val;
    }
}

class TestVcfCompare : public ::testing::Test {
protected:
    void SetUp() {
        stringstream hdrss(headerText);
        InputStream in("test", hdrss);
        _header = Header::fromStream(in);
    }

    Header _header;
};

TEST_F(TestVcfCompare, altIntersectDifferentChrom) {
    string v1 = "1\t10\t.\tG\tA,C\t.\t.\t.\t.";
    string v2 = "2\t10\t.\tG\tC,T\t.\t.\t.\t.";
    Entry e1(&_header, v1);
    Entry e2(&_header, v2);
    Compare::AltIntersect xsec;
    auto result = xsec(e1, e2);
    ASSERT_TRUE(result.empty());
}

TEST_F(TestVcfCompare, altIntersectDifferentPos) {
    string v1 = "1\t10\t.\tG\tA,C\t.\t.\t.\t.";
    string v2 = "1\t11\t.\tG\tC,T\t.\t.\t.\t.";
    Entry e1(&_header, v1);
    Entry e2(&_header, v2);
    Compare::AltIntersect xsec;
    auto result = xsec(e1, e2);
    ASSERT_TRUE(result.empty());
}

TEST_F(TestVcfCompare, altIntersectSnv1) {
    string v1 = "1\t10\t.\tG\tA,C\t.\t.\t.\t.";
    string v2 = "1\t10\t.\tG\tC,T\t.\t.\t.\t.";
    Entry e1(&_header, v1);
    Entry e2(&_header, v2);
    Compare::AltIntersect xsec;
    auto result = xsec(e1, e2);
    ASSERT_EQ(1, result.size());
    ASSERT_TRUE(mapHasPair(result, 1, 0));
}

TEST_F(TestVcfCompare, altIntersectSnv2) {
    string v1 = "1\t10\t.\tG\tA,C\t.\t.\t.\t.";
    string v2 = "1\t10\t.\tG\tC,A\t.\t.\t.\t.";
    Entry e1(&_header, v1);
    Entry e2(&_header, v2);
    Compare::AltIntersect xsec;
    auto result = xsec(e1, e2);
    ASSERT_EQ(2, result.size());
    ASSERT_TRUE(mapHasPair(result, 0, 1));
    ASSERT_TRUE(mapHasPair(result, 1, 0));
}

TEST_F(TestVcfCompare, altIntersectIndel1) {
    string v1 = "1\t10\t.\tGAC\tGA,G\t.\t.\t.\t.";
    string v2 = "1\t10\t.\tGAC\tG\t.\t.\t.\t.";
    Entry e1(&_header, v1);
    Entry e2(&_header, v2);
    Compare::AltIntersect xsec;
    auto result = xsec(e1, e2);
    ASSERT_EQ(1, result.size());
    ASSERT_TRUE(mapHasPair(result, 1, 0));
}
