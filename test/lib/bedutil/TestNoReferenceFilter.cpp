#include "bedutil/NoReferenceFilter.hpp"
#include "fileformats/Bed.hpp"

#include <boost/assign/list_of.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using boost::assign::list_of;
using namespace std;

TEST(NoReferenceFilter, exclude) {
    string baseLine("1\t2\t3\t");
    vector<string> excludes = list_of("A/")("\0\0")("   ")("NNN");

    BedHeader hdr;
    NoReferenceFilter filter;
    for (unsigned i = 0; i < excludes.size(); ++i) {
        Bed snv;
        string bad = baseLine + excludes[i];
        Bed::parseLine(&hdr, bad, snv, 1);
        ASSERT_TRUE(filter.exclude(snv));
    }

    const char* valid = "TACG";
    while (*valid) {
        Bed snv;
        string line = baseLine + *valid + string("/C");
        Bed::parseLine(&hdr, line, snv, 1);
        ASSERT_FALSE(filter.exclude(snv)) <<
            "don't exclude '" << *valid << "' as reference value (" << line << ")";
        ++valid;
    }
    ASSERT_EQ(4u, filter.filtered());
}
