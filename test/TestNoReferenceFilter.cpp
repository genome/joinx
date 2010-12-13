#include "NoReferenceFilter.hpp"
#include "Bed.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(NoReferenceFilter, exclude) {
    NoReferenceFilter filter;
    Bed snp;

    snp.refCall = "A/";
    ASSERT_TRUE(filter.exclude(snp)) << "exclude short values of refCall";
    snp.refCall = "\0\0\0";
    ASSERT_TRUE(filter.exclude(snp)) << "exclude '\0' as reference value";
    snp.refCall = "   ";
    ASSERT_TRUE(filter.exclude(snp)) << "exclude ' ' as reference value";
    snp.refCall = "NNN";
    ASSERT_TRUE(filter.exclude(snp)) << "exclude 'N' as reference value";

    const char* valid = "TACG";
    while (*valid) {
        snp.refCall = (*valid++) + string("/C");
        ASSERT_FALSE(filter.exclude(snp)) << 
            "don't exclude '" << snp.refCall << "' as reference value";
    }
    ASSERT_EQ(4, filter.filtered());
}
