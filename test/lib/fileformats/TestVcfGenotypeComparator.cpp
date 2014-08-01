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
#include <memory>
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
                if (i->ref.empty() && i->alt.empty())
                    ss << "REF";
                else
                    ss << i->pos << " " << i->ref << "->" << i->alt;

                callStrings.push_back(ss.str());
            }
            std::stringstream ss;
            ss << "S" << sampleIdx << " " << sequence << " "
                << streamJoin(callStrings).delimiter(",");

            std::vector<size_t> fileIndices(which.size());
            size_t idx = 0;
            for (auto i = which.begin(); i != which.end(); ++i, ++idx) {
                fileIndices[idx] = i->first;
            }

            calls[ss.str()] = streamJoin(fileIndices).delimiter(",").toString();
        }

        std::map<std::string, std::string> calls;
    };

    template<typename OS>
    OS& operator<<(OS& os, Collector const& collector) {
        for (auto iter = collector.calls.begin(); iter != collector.calls.end(); ++iter) {
            os << iter->first << ": " << iter->second << "\n";
        }
        return os;
    }



}

class TestVcfGenotypeComparator : public ::testing::Test {
public:
    void SetUp() {
        std::string header(
            "##fileformat=VCFv4.1\n"
            "##FORMAT=<ID=GT,Type=String,Number=1,Description=\"Genotype\">\n"
            "##FORMAT=<ID=FT,Type=String,Number=1,Description=\"Genotype filter\">\n"
            "##FILTER=<ID=FAIL,Description=\"Not so great\">\n"
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tS1\tS2\n"
            );

        sampleNames_.push_back("S1");
        sampleNames_.push_back("S2");

        for (size_t i = 0; i < nStreams; ++i) {
            std::stringstream ss(header);
            // we happen to need pointers, so copy to heap
            headers_.push_back(new Vcf::Header(Vcf::Header::fromStream(ss)));
            headers_.back()->sourceIndex(i);
            filterTypes_.push_back(Vcf::eUNFILTERED);
        }

        gcmp.reset(new Vcf::GenotypeComparator<Collector>(sampleNames_, headers_, filterTypes_, nStreams, collector));
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
            std::string const& gtdata = ".",
            bool siteFilter = false
            )
    {
        stringstream ss;
        ss << chrom << "\t" << pos << "\t.\t" << ref << "\t" << alt << "\t.\t"
            << (siteFilter ? "FAIL" : ".") <<
            "\t.\tGT:FT\t" << gtdata;

        return new Vcf::Entry(headers_[headerIdx], ss.str());
    }

    Vcf::Entry* makeFilterEntry(
            size_t headerIdx,
            std::string chrom,
            int64_t pos,
            std::string const& ref,
            std::string const& alt,
            std::string const& gtdata,
            bool siteFilter,
            bool sampleFilter
            )
    {
        if (gtdata.find("\t") != std::string::npos) { 
            throw std::runtime_error("Can't use makeFilterEntry with multiple samples!");
        }

        std::string newGtData = gtdata;
        if (sampleFilter) {
            newGtData += ":FAIL";
        }
        return makeEntry(headerIdx, chrom, pos, ref, alt, newGtData, siteFilter);
    }

protected:
    std::vector<std::string> sampleNames_;
    std::vector<Vcf::Header*> headers_;
    std::vector<Vcf::FilterType> filterTypes_;
    Collector collector;
    std::unique_ptr<Vcf::GenotypeComparator<Collector>> gcmp;
};

TEST_F(TestVcfGenotypeComparator, homRefSkipped) {
    gcmp->push(makeEntry(0, "3", 20, "A", "A", "0/0"));
    gcmp->push(makeEntry(1, "3", 20, "A", "A", "0/0"));

    gcmp->finalize();

    EXPECT_TRUE(collector.calls.empty());
}

TEST_F(TestVcfGenotypeComparator, process) {
    gcmp->push(makeEntry(0, "1", 10, "A", "G", "1/1\t0/1"));
    gcmp->push(makeEntry(2, "1", 10, "A", "G", "1/1\t0/1"));
    gcmp->push(makeEntry(1, "1", 10, "A", "G", "0/1\t1/1"));
    gcmp->push(makeEntry(2, "1", 10, "A", "C", "0/1"));

    gcmp->push(makeEntry(2, "1", 11, "A", "G", "0/1"));
    gcmp->push(makeEntry(0, "1", 11, "A", "G", "0/1"));

    gcmp->push(makeEntry(0, "1", 12, "A", "G,C", "1/1\t1/2"));

    gcmp->push(makeEntry(0, "1", 12, "AA", "A,C", "1/1\t1/2"));

    gcmp->push(makeEntry(0, "2", 10, "AA", "AG", "0|1"));
    gcmp->push(makeEntry(1, "2", 11, "A", "G", "1|0"));

    gcmp->push(makeEntry(0, "3", 10, "A", "A", "0/0"));
    gcmp->push(makeEntry(1, "3", 10, "A", "A", "0/0"));

    gcmp->finalize();

    size_t cnt = 0;

    EXPECT_EQ(  "2", collector.calls["S0 1 10 A->C,REF"]); ++cnt;
    EXPECT_EQ(  "1", collector.calls["S0 1 10 A->G,REF"]); ++cnt;
    EXPECT_EQ("0,2", collector.calls["S0 1 10 A->G,10 A->G"]); ++cnt;
    EXPECT_EQ(  "1", collector.calls["S1 1 10 A->G,10 A->G"]); ++cnt;

    EXPECT_EQ("0,1", collector.calls["S0 2 11 A->G,REF"]); ++cnt;
    EXPECT_EQ("0,2", collector.calls["S1 1 10 A->G,REF"]); ++cnt;


    EXPECT_EQ("0,2", collector.calls["S0 1 11 A->G,REF"]); ++cnt;

    EXPECT_EQ(  "0", collector.calls["S0 1 12 A->G,12 A->G"]); ++cnt;
    EXPECT_EQ(  "0", collector.calls["S1 1 12 A->C,12 A->G"]); ++cnt;

    EXPECT_EQ(  "0", collector.calls["S0 1 13 A->,13 A->"]); ++cnt;
    EXPECT_EQ(  "0", collector.calls["S1 1 12 AA->C,13 A->"]); ++cnt;

    EXPECT_EQ(cnt, collector.calls.size());
}

TEST_F(TestVcfGenotypeComparator, siteFilter) {
    auto unfiltered = makeEntry(0, "1", 10, "A", "G", "1/1\t0/1");
    auto filtered = makeEntry(1, "1", 10, "A", "G", "1/1\t0/1");
    auto gtfiltered = makeEntry(2, "1", 10, "A", "G", "1/1:FalsePositive\t0/1");

    filtered->addFilter("FalsePositive");

    EXPECT_TRUE(filtered->isFiltered());
    EXPECT_FALSE(unfiltered->isFiltered());
    EXPECT_FALSE(gtfiltered->isFiltered());
    EXPECT_TRUE(gtfiltered->sampleData().isSampleFiltered(0));

    gcmp->push(filtered);
    gcmp->push(unfiltered);
    gcmp->push(gtfiltered);
    gcmp->finalize();

    // 2 doesn't show up because it is GT filtered
    EXPECT_EQ(  "0", collector.calls["S0 1 10 A->G,10 A->G"]);
    EXPECT_EQ("0,2", collector.calls["S1 1 10 A->G,REF"]);
}

TEST_F(TestVcfGenotypeComparator, filteringOptions_Both) {
    using namespace Vcf;
    std::vector<FilterType> customFilterTypes{eBOTH, eFILTERED, eUNFILTERED};
    auto gcmp2 = makeGenotypeComparator(sampleNames_, headers_, customFilterTypes, nStreams, collector);

    bool T = true;
    bool F = false;

    gcmp2.push(makeFilterEntry(0, "1", 10, "A", "C", "0/1", F, F));
    gcmp2.push(makeFilterEntry(1, "1", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "2", 10, "A", "C", "0/1", F, T));
    gcmp2.push(makeFilterEntry(1, "2", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "3", 10, "A", "C", "0/1", T, F));
    gcmp2.push(makeFilterEntry(1, "3", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "4", 10, "A", "C", "0/1", T, T));
    gcmp2.push(makeFilterEntry(1, "4", 10, "A", "C", "0/1", T, T));

    gcmp2.finalize();

    EXPECT_EQ("0,1", collector.calls["S0 1 10 A->C,REF"]);
    EXPECT_EQ("0,1", collector.calls["S0 2 10 A->C,REF"]);
    EXPECT_EQ("0,1", collector.calls["S0 3 10 A->C,REF"]);
    EXPECT_EQ("0,1", collector.calls["S0 4 10 A->C,REF"]);
}

TEST_F(TestVcfGenotypeComparator, filteringOptions_Filtered) {
    using namespace Vcf;
    std::vector<FilterType> customFilterTypes{eFILTERED, eBOTH, eUNFILTERED};
    auto gcmp2 = makeGenotypeComparator(sampleNames_, headers_, customFilterTypes, nStreams, collector);

    bool T = true;
    bool F = false;

    gcmp2.push(makeFilterEntry(0, "1", 10, "A", "C", "0/1", F, F));
    gcmp2.push(makeFilterEntry(1, "1", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "2", 10, "A", "C", "0/1", F, T));
    gcmp2.push(makeFilterEntry(1, "2", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "3", 10, "A", "C", "0/1", T, F));
    gcmp2.push(makeFilterEntry(1, "3", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "4", 10, "A", "C", "0/1", T, T));
    gcmp2.push(makeFilterEntry(1, "4", 10, "A", "C", "0/1", T, T));

    gcmp2.finalize();

    EXPECT_EQ(  "1", collector.calls["S0 1 10 A->C,REF"]);
    EXPECT_EQ("0,1", collector.calls["S0 2 10 A->C,REF"]);
    EXPECT_EQ("0,1", collector.calls["S0 3 10 A->C,REF"]);
    EXPECT_EQ("0,1", collector.calls["S0 4 10 A->C,REF"]);
}

TEST_F(TestVcfGenotypeComparator, filteringOptions_UnFiltered) {
    using namespace Vcf;
    std::vector<FilterType> customFilterTypes{eUNFILTERED, eBOTH, eFILTERED};
    auto gcmp2 = makeGenotypeComparator(sampleNames_, headers_, customFilterTypes, nStreams, collector);

    bool T = true;   // filtered
    bool F = false;  // unfiltered

    gcmp2.push(makeFilterEntry(0, "1", 10, "A", "C", "0/1", F, F));
    gcmp2.push(makeFilterEntry(1, "1", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "2", 10, "A", "C", "0/1", F, T));
    gcmp2.push(makeFilterEntry(1, "2", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "3", 10, "A", "C", "0/1", T, F));
    gcmp2.push(makeFilterEntry(1, "3", 10, "A", "C", "0/1", T, T));

    gcmp2.push(makeFilterEntry(0, "4", 10, "A", "C", "0/1", T, T));
    gcmp2.push(makeFilterEntry(1, "4", 10, "A", "C", "0/1", T, T));

    gcmp2.finalize();

    EXPECT_EQ("0,1", collector.calls["S0 1 10 A->C,REF"]);
    EXPECT_EQ(  "1", collector.calls["S0 2 10 A->C,REF"]);
    EXPECT_EQ(  "1", collector.calls["S0 3 10 A->C,REF"]);
    EXPECT_EQ(  "1", collector.calls["S0 4 10 A->C,REF"]);
}
