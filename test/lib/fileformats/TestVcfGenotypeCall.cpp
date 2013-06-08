#include "fileformats/vcf/GenotypeCall.hpp"

#include <gtest/gtest.h>
#include <iterator>
#include <stdexcept>
#include <string>

using namespace std;
using namespace Vcf;

TEST(GenotypeCall, empty) {
    GenotypeCall gt;
    ASSERT_TRUE(gt.empty());
    ASSERT_EQ(0u, gt.size());
    ASSERT_EQ(0u, distance(gt.begin(), gt.end()));
    ASSERT_FALSE(gt.phased());
    ASSERT_TRUE(gt.indices().empty());
}

TEST(GenotypeCall, emptyString) {
    GenotypeCall gt("");
    ASSERT_TRUE(gt.empty());
    ASSERT_EQ(0u, gt.size());
    ASSERT_EQ(0u, distance(gt.begin(), gt.end()));
    ASSERT_FALSE(gt.phased());
    ASSERT_TRUE(gt.indices().empty());
}

TEST(GenotypeCall, phased) {
    GenotypeCall gt("0|1");
    ASSERT_TRUE(gt.phased());
    ASSERT_FALSE(gt.empty());
    ASSERT_EQ(2u, gt.size());
    ASSERT_EQ(2u, distance(gt.begin(), gt.end()));
    ASSERT_EQ(0u, gt[0]);
    ASSERT_EQ(1u, gt[1]);
}

TEST(GenotypeCall, unphased) {
    GenotypeCall gt("0/1/2/3");
    ASSERT_FALSE(gt.phased());
    ASSERT_FALSE(gt.empty());
    ASSERT_EQ(4u, gt.size());
    ASSERT_EQ(4u, distance(gt.begin(), gt.end()));
    ASSERT_EQ(0u, gt[0]);
    ASSERT_EQ(1u, gt[1]);
    ASSERT_EQ(2u, gt[2]);
    ASSERT_EQ(3u, gt[3]);
}
