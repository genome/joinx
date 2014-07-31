#include "annotate/HomopolymerAnnotator.hpp"

#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "io/StreamJoin.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace {
    typedef std::set<int64_t> AltIndices;

    // Indices are 0-based indices into the alternate alleles array
    ::testing::AssertionResult theHomopolymersAre(Vcf::Entry const& entry, AltIndices indices) {
        auto const* info = entry.info("HOMOPOLYMER");

        // No homopolymer tag. Succeed only if indices is empty
        if (!info && !indices.empty()) {
            return ::testing::AssertionFailure()
                << "No homopolymer annotations! We expected alts " << streamJoin(indices);
        }

        for (size_t i = 0; i < info->size(); ++i) {
            int64_t const* val = info->get<int64_t>(i);
            if (!val || *val == 0)
                continue;

            // at this point, index i was annotated
            auto iter = indices.find(i);
            if (iter == indices.end())
                return ::testing::AssertionFailure()
                    << "Alternate allele " << i << " (" << entry.alt()[i]
                    << ") was not expected to be annotated.";

            indices.erase(iter);
        }

        if (!indices.empty())
            return ::testing::AssertionFailure()
                << "Expected the following alts to be annotated: " << streamJoin(indices);

        return ::testing::AssertionSuccess();
    }


    std::stringstream hdrss(
            "##fileformat=VCFv4.1\n"
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\n"
            );

    struct Collector {
        void operator()(Vcf::Entry&& e) {
            entries.push_back(std::move(e));
        }

        std::vector<Vcf::Entry> entries;
    };
}

class TestHomopolymerAnnotator : public ::testing::Test {
public:
    TestHomopolymerAnnotator()
        : infoType_(0)
    {}

    void SetUp() {
        header_ = Vcf::Header::fromStream(hdrss);
        Vcf::CustomType infoType("HOMOPOLYMER", Vcf::CustomType::PER_ALLELE,
            0, Vcf::CustomType::INTEGER, "Per-allele homopolymer status");
        header_.addInfoType(infoType);
        infoType_ = header_.infoType("HOMOPOLYMER");
    }

    Vcf::Entry makeEntry(std::string const& chrom, int64_t pos, std::string const& ref, std::string const& alts) {
        std::stringstream line;
        line << chrom << "\t" << pos << "\t.\t" << ref << "\t" << alts
                << "\t.\t.\t.\t.";
        return Vcf::Entry(&header_, line.str());
    }

protected:
    Collector collector;
    Vcf::Header header_;
    Vcf::CustomType const* infoType_;
};

TEST_F(TestHomopolymerAnnotator, adjacentPreInsertions) {
    size_t maxLength = 2;
    auto annotator = makeHomopolymerAnnotator(collector, maxLength, infoType_);

    // Only the first two should be marked
    Bed homop("1", 10, 15, std::vector<std::string>{"A"});
    auto vcf = makeEntry("1", 10, "T", "TA,TAA,TAAA,TT,TTT,C,TT");
    annotator.hit(homop, vcf);
    annotator.flush();

    ASSERT_EQ(1u, collector.entries.size());
    auto const& entry = collector.entries[0];
    EXPECT_TRUE(theHomopolymersAre(entry, AltIndices{0, 1}));

    // Now move back one position and we should not get annotations
    collector.entries.clear();
    vcf = makeEntry("1", 9, "T", "TA");
    annotator.hit(homop, vcf); // note the intersector wouldn't actually do this
    annotator.flush();
    EXPECT_TRUE(theHomopolymersAre(entry, AltIndices{}));
}

TEST_F(TestHomopolymerAnnotator, adjacentPostInsertions) {
    size_t maxLength = 2;
    auto annotator = makeHomopolymerAnnotator(collector, maxLength, infoType_);

    Bed homop("1", 10, 15, std::vector<std::string>{"A"});
    auto vcf = makeEntry("1", 15, "A", "AT,AA,AAA,ACA,AAAA");
    annotator.hit(homop, vcf);
    annotator.flush();

    ASSERT_EQ(1u, collector.entries.size());
    auto const& entry = collector.entries[0];
    EXPECT_TRUE(theHomopolymersAre(entry, AltIndices{1, 2}));
}
