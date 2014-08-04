#include "fileformats/vcf/GenotypeCall.hpp"

#include <boost/lexical_cast.hpp>
#include <gtest/gtest.h>

#include <iterator>
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

    EXPECT_EQ(".", boost::lexical_cast<std::string>(gt));
}

TEST(GenotypeCall, emptyString) {
    GenotypeCall gt("");
    EXPECT_TRUE(gt.empty());
    EXPECT_EQ(0u, gt.size());
    EXPECT_EQ(0u, distance(gt.begin(), gt.end()));
    EXPECT_FALSE(gt.phased());
    EXPECT_TRUE(gt.indices().empty());

    EXPECT_EQ(".", boost::lexical_cast<std::string>(gt));
}

TEST(GenotypeCall, nullString) {
    GenotypeCall gt(".");
    EXPECT_TRUE(gt.empty());
    EXPECT_EQ(0u, gt.size());
    EXPECT_EQ(0u, distance(gt.begin(), gt.end()));
    EXPECT_FALSE(gt.phased());
    EXPECT_TRUE(gt.indices().empty());

    EXPECT_EQ(".", boost::lexical_cast<std::string>(gt));
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

    EXPECT_EQ("0|1", boost::lexical_cast<std::string>(gt));
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

    EXPECT_EQ("0/1/2/3", boost::lexical_cast<std::string>(gt));
}

void testMissing(std::string const& data, std::vector<GenotypeIndex> const& expected) {
    GenotypeCall gt(data);

    EXPECT_TRUE(gt.diploid());
    EXPECT_EQ(2u, gt.size());
    EXPECT_EQ(2u, gt.indices().size());
    EXPECT_EQ(2u, gt.indexSet().size());
    EXPECT_EQ(1u, gt.indexSet().count(GenotypeIndex::Null));
    EXPECT_EQ(1u, gt.indexSet().count(GenotypeIndex{1}));

    EXPECT_TRUE(gt.partial());
    EXPECT_FALSE(gt.null());
    EXPECT_FALSE(gt.empty());
    EXPECT_FALSE(gt.heterozygous());
    EXPECT_FALSE(gt.homozygous());

    EXPECT_EQ(gt.indices(), expected);
    EXPECT_EQ(data, boost::lexical_cast<std::string>(gt));
}


TEST(GenotypeCall, missingData) {
    {
        SCOPED_TRACE("Missing gt: ./1");
        testMissing("./1", std::vector<GenotypeIndex>{GenotypeIndex::Null, GenotypeIndex{1}});
    }

    {
        SCOPED_TRACE("Missing gt: 1/.");
        testMissing("1/.", std::vector<GenotypeIndex>{GenotypeIndex{1}, GenotypeIndex::Null});
    }
}

TEST(GenotypeCall, bothNull) {
    GenotypeCall gt("./.");
    EXPECT_TRUE(gt.diploid());
    EXPECT_TRUE(gt.null());
    EXPECT_FALSE(gt.empty());
    EXPECT_FALSE(gt.homozygous());
    EXPECT_FALSE(gt.heterozygous());

    EXPECT_EQ("./.", boost::lexical_cast<std::string>(gt));
}

TEST(GenotypeCall, genotypeIndex) {
    std::stringstream ss;
    GenotypeIndex idx{1};
    ss << idx << '/' << GenotypeIndex::Null;
    EXPECT_EQ("1/.", ss.str());
    EXPECT_TRUE(idx == 1u);
}
