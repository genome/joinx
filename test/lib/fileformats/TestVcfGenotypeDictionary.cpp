#include "fileformats/vcf/GenotypeDictionary.hpp"

#include "fileformats/vcf/RawVariant.hpp"

#include <gtest/gtest.h>

namespace {
    typedef Vcf::RawVariant::Vector Genotype;
}

class TestVcfGenotypeDictionary : public ::testing::Test {
public:
    void SetUp() {
        hetSnv.push_back(new Vcf::RawVariant(10, "A", "A"));
        hetSnv.push_back(new Vcf::RawVariant(10, "A", "C"));

        homSnv.push_back(new Vcf::RawVariant(10, "A", "C"));
        homSnv.push_back(new Vcf::RawVariant(10, "A", "C"));
    }

protected:
    Genotype hetSnv;
    Genotype homSnv;
};

TEST_F(TestVcfGenotypeDictionary, homozygousSnv) {
    Vcf::GenotypeDictionary<unsigned> dict;
    dict.add(homSnv, 1);
    dict.add(homSnv, 2);
    dict.add(homSnv, 3);

    auto exactHom = dict.exactMatches(homSnv);
    EXPECT_EQ(3u, exactHom.size());
    EXPECT_EQ(1u, exactHom.count(1));
    EXPECT_EQ(1u, exactHom.count(2));
    EXPECT_EQ(1u, exactHom.count(3));

    auto exactHet = dict.exactMatches(hetSnv);
    EXPECT_TRUE(exactHet.empty());

    auto partialRef = dict.allMatches(hetSnv[0]);
    // The dictionary doesn't have any copies of the reference allele
    // (we added a homozygous snv)
    EXPECT_TRUE(partialRef.empty());

    auto partialVar = dict.allMatches(hetSnv[1]);
    // Each key (1,2,3) should report two copies of the variant allele
    EXPECT_EQ(3u, partialVar.size());
    EXPECT_EQ(2u, partialVar[1]);
    EXPECT_EQ(2u, partialVar[2]);
    EXPECT_EQ(2u, partialVar[3]);
}

TEST_F(TestVcfGenotypeDictionary, heterozygousSnv) {
    Vcf::GenotypeDictionary<unsigned> dict;
    dict.add(hetSnv, 1);
    dict.add(hetSnv, 2);
    dict.add(hetSnv, 3);

    auto exactHet = dict.exactMatches(hetSnv);
    EXPECT_EQ(3u, exactHet.size());
    EXPECT_EQ(1u, exactHet.count(1));
    EXPECT_EQ(1u, exactHet.count(2));
    EXPECT_EQ(1u, exactHet.count(3));

    auto exactHom = dict.exactMatches(homSnv);
    EXPECT_TRUE(exactHom.empty());

    // The dictionary should have three copies each of the reference
    // and variant allele
    auto partialRef = dict.allMatches(hetSnv[0]);
    EXPECT_EQ(3u, partialRef.size());
    EXPECT_EQ(1u, partialRef[1]);
    EXPECT_EQ(1u, partialRef[2]);
    EXPECT_EQ(1u, partialRef[3]);

    auto partialVar = dict.allMatches(hetSnv[1]);
    // Each key (1,2,3) should report two copies of the variant allele
    EXPECT_EQ(3u, partialVar.size());
    EXPECT_EQ(1u, partialVar[1]);
    EXPECT_EQ(1u, partialVar[2]);
    EXPECT_EQ(1u, partialVar[3]);
}

TEST_F(TestVcfGenotypeDictionary, mixedZygositySnvs) {
    Vcf::GenotypeDictionary<unsigned> dict;
    dict.add(hetSnv, 1);
    dict.add(homSnv, 2);
    dict.add(hetSnv, 3);

    auto exactHet = dict.exactMatches(hetSnv);
    EXPECT_EQ(2u, exactHet.size());
    EXPECT_EQ(1u, exactHet.count(1));
    EXPECT_EQ(0u, exactHet.count(2)); // #2 is homozygous
    EXPECT_EQ(1u, exactHet.count(3));

    auto exactHom = dict.exactMatches(homSnv);
    EXPECT_EQ(1u, exactHom.size());
    EXPECT_EQ(1u, exactHom.count(2));

    auto partialRef = dict.allMatches(hetSnv[0]);
    EXPECT_EQ(2u, partialRef.size());
    EXPECT_EQ(1u, partialRef[1]);
    EXPECT_EQ(0u, partialRef.count(2));
    EXPECT_EQ(1u, partialRef[3]);

    auto partialVar = dict.allMatches(hetSnv[1]);
    EXPECT_EQ(3u, partialVar.size());
    EXPECT_EQ(1u, partialVar[1]);
    EXPECT_EQ(2u, partialVar[2]);
    EXPECT_EQ(1u, partialVar[3]);
}
