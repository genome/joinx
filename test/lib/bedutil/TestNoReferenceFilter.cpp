#include "bedutil/NoReferenceFilter.hpp"
#include "bedutil/Bed.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(NoReferenceFilter, exclude) {
    NoReferenceFilter filter;
    Bed snv;

    snv.refCall = "A/";
    ASSERT_TRUE(filter.exclude(snv)) << "exclude short values of refCall";
    snv.refCall = "\0\0\0";
    ASSERT_TRUE(filter.exclude(snv)) << "exclude '\0' as reference value";
    snv.refCall = "   ";
    ASSERT_TRUE(filter.exclude(snv)) << "exclude ' ' as reference value";
    snv.refCall = "NNN";
    ASSERT_TRUE(filter.exclude(snv)) << "exclude 'N' as reference value";

    const char* valid = "TACG";
    while (*valid) {
        snv.refCall = (*valid++) + string("/C");
        ASSERT_FALSE(filter.exclude(snv)) << 
            "don't exclude '" << snv.refCall << "' as reference value";
    }
    ASSERT_EQ(4u, filter.filtered());
}
