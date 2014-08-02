#include "fileformats/vcf/GenotypeCall.hpp"

#include <gtest/gtest.h>

#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace Vcf;

TEST(GenotypeCall, empty) {
    GenotypeCall gt;
    EXPECT_TRUE(gt.empty());
    EXPECT_EQ(0u, gt.size());
    EXPECT_EQ(0u, distance(gt.begin(), gt.end()));
    EXPECT_FALSE(gt.phased());
    EXPECT_TRUE(gt.indices().empty());
}

TEST(GenotypeCall, emptyString) {
    GenotypeCall gt("");
    EXPECT_TRUE(gt.empty());
    EXPECT_EQ(0u, gt.size());
    EXPECT_EQ(0u, distance(gt.begin(), gt.end()));
    EXPECT_FALSE(gt.phased());
    EXPECT_TRUE(gt.indices().empty());
}

TEST(GenotypeCall, phased) {
    GenotypeCall gt("0|1");
    EXPECT_TRUE(gt.phased());
    EXPECT_TRUE(gt.diploid());
    EXPECT_FALSE(gt.empty());
    EXPECT_EQ(2u, gt.size());
    EXPECT_EQ(2u, distance(gt.begin(), gt.end()));
    EXPECT_EQ(0u, gt[0]);
    EXPECT_EQ(1u, gt[1]);
}

TEST(GenotypeCall, unphased) {
    GenotypeCall gt("0/1/2/3");
    EXPECT_FALSE(gt.phased());
    EXPECT_FALSE(gt.empty());
    EXPECT_FALSE(gt.diploid());
    EXPECT_EQ(4u, gt.size());
    EXPECT_EQ(4u, distance(gt.begin(), gt.end()));
    EXPECT_EQ(0u, gt[0]);
    EXPECT_EQ(1u, gt[1]);
    EXPECT_EQ(2u, gt[2]);
    EXPECT_EQ(3u, gt[3]);
}

TEST(GenotypeCall, missingData) {
    GenotypeCall gt("./1");
    EXPECT_TRUE(gt.diploid());
    EXPECT_EQ(2u, gt.size());
    EXPECT_EQ(GenotypeIndex::Null, gt[0]);
    EXPECT_EQ(1, gt[1].value);

    EXPECT_EQ(gt, gt);
    EXPECT_EQ(2u, gt.indices().size());
    EXPECT_EQ(2u, gt.indexSet().size());

    std::stringstream ss;
    ss << gt;
    EXPECT_EQ("./1", ss.str());

    EXPECT_FALSE(gt.homozygous());
    EXPECT_TRUE(gt.heterozygous());
}

TEST(GenotypeCall, genotypeIndex) {
    std::stringstream ss;
    GenotypeIndex idx{1};
    ss << idx << '/' << GenotypeIndex::Null;
    EXPECT_EQ("1/.", ss.str());
    EXPECT_TRUE(idx == 1u);
}
