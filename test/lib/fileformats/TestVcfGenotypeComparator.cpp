#include "fileformats/vcf/GenotypeComparator.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "common/cstdint.hpp"
#include "io/StreamJoin.hpp"

#include <gtest/gtest.h>
#include <boost/ref.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
    size_t nStreams = 3;

    struct Collector {
        void operator()(
                size_t sampleIdx,
                std::string const& sequence,
                Vcf::RawVariant::Vector const& vars,
                std::map<size_t, Vcf::Entry const*> const& which)
        {
            std::vector<std::string> callStrings;
            for (auto i = vars.begin(); i != vars.end(); ++i) {
                std::stringstream ss;
                ss << i->pos << " " << i->ref << "->" << i->alt;

                callStrings.push_back(ss.str());
            }
            std::stringstream ss;
            ss << "S" << sampleIdx << " " << sequence << " "
                << streamJoin(callStrings).delimiter(",");

            calls[ss.str()] = streamJoin(which).delimiter(",").toString();
        }

        std::map<std::string, std::string> calls;
    };

}

class TestVcfGenotypeComparator : public ::testing::Test {
public:
    void SetUp() {
        std::string header(
            "##fileformat=VCFv4.1\n"
            "##FORMAT=<ID=GT,Type=String,Number=1,Description=\"Genotype\">\n"
            "##FORMAT=<ID=FT,Type=String,Number=1,Description=\"Genotype filter\">\n"
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\tS2\n"
            );

        sampleNames_.push_back("S1");
        sampleNames_.push_back("S2");

        for (size_t i = 0; i < nStreams; ++i) {
            std::stringstream ss(header);
            // we happen to need pointers, so copy to heap
            headers_.push_back(new Vcf::Header(Vcf::Header::fromStream(ss)));
            headers_.back()->sourceIndex(i);
        }
    }

    void TearDown() {
        for (auto x = headers_.begin(); x != headers_.end(); ++x) {
            delete *x;
        }
    }


    Vcf::Entry* makeEntry(
            size_t headerIdx,
            std::string chrom,
            int64_t pos,
            std::string const& ref,
            std::string const& alt,
            std::string const& gt = "."
            )
    {
        stringstream ss;
        ss << chrom << "\t" << pos << "\t.\t" << ref << "\t" << alt << "\t.\t.\t.\tGT:FT\t" << gt;
        return new Vcf::Entry(headers_[headerIdx], ss.str());
    }

protected:
    std::vector<std::string> sampleNames_;
    std::vector<Vcf::Header*> headers_;
};

TEST_F(TestVcfGenotypeComparator, process) {
    Collector c;
    auto gcmp = Vcf::makeGenotypeComparator(sampleNames_, headers_, nStreams, c);
    gcmp.push(makeEntry(0, "1", 10, "A", "G", "1/1\t0/1"));
    gcmp.push(makeEntry(2, "1", 10, "A", "G", "1/1\t0/1"));
    gcmp.push(makeEntry(1, "1", 10, "A", "G", "0/1\t1/1"));
    gcmp.push(makeEntry(2, "1", 10, "A", "C", "0/1"));

    gcmp.push(makeEntry(2, "1", 11, "A", "G", "0/1"));
    gcmp.push(makeEntry(0, "1", 11, "A", "G", "0/1"));

    gcmp.push(makeEntry(0, "1", 12, "A", "G,C", "1/1\t1/2"));

    gcmp.push(makeEntry(0, "1", 12, "AA", "A,C", "1/1\t1/2"));
    gcmp.finalize();

    EXPECT_EQ("0,2", c.calls["S0 1 10 A->G,10 A->G"]);
    EXPECT_EQ("0,2", c.calls["S1 1 10 A->A,10 A->G"]);
    EXPECT_EQ(  "1", c.calls["S0 1 10 A->A,10 A->G"]);

    EXPECT_EQ(  "2", c.calls["S0 1 10 A->A,10 A->C"]);

    EXPECT_EQ("0,2", c.calls["S0 1 11 A->A,11 A->G"]);

    EXPECT_EQ(  "0", c.calls["S0 1 12 A->G,12 A->G"]);
    EXPECT_EQ(  "0", c.calls["S1 1 12 A->C,12 A->G"]);

    EXPECT_EQ(  "0", c.calls["S0 1 13 A->,13 A->"]);
    EXPECT_EQ(  "0", c.calls["S1 1 12 AA->C,13 A->"]);
}

TEST_F(TestVcfGenotypeComparator, siteFilter) {
    Collector c;
    auto gcmp = Vcf::makeGenotypeComparator(sampleNames_, headers_, nStreams, c);

    auto unfiltered = makeEntry(0, "1", 10, "A", "G", "1/1\t0/1");
    auto filtered = makeEntry(1, "1", 10, "A", "G", "1/1\t0/1");
    auto gtfiltered = makeEntry(2, "1", 10, "A", "G", "1/1:FalsePositive\t0/1");

    filtered->addFilter("FalsePositive");

    EXPECT_TRUE(filtered->isFiltered());
    EXPECT_FALSE(unfiltered->isFiltered());
    EXPECT_FALSE(gtfiltered->isFiltered());
    EXPECT_TRUE(gtfiltered->sampleData().isSampleFiltered(0));

    gcmp.push(filtered);
    gcmp.push(unfiltered);
    gcmp.push(gtfiltered);
    gcmp.finalize();
}
