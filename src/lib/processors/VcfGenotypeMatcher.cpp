#include "VcfGenotypeMatcher.hpp"

#include "common/Integer.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Header.hpp"
#include "io/StreamJoin.hpp"

#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace bfs = boost::filesystem;
using namespace Vcf;
using boost::container::flat_set;
using boost::format;


namespace {
    CustomType const* getType(Vcf::Header const& header, std::string const& id) {
        CustomType const* type = header.formatType(id);
        if (!type) {
            throw std::runtime_error(str(format(
                "Format field %1% not found in vcf header for file %2%"
                ) % id % header.sourceIndex()));
        }
        return type;
    }

    template<typename VecType, typename SetType>
    void setValues(VecType& v, SetType const& s) {
        for (auto i = s.begin(); i != s.end(); ++i) {
            assert(v.size() > *i);
            v[*i] = int64_t{1};
        }
    }

    template<typename Container>
    Container difference_(Container const& s1, Container const& s2) {
        Container rv;
        std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
            std::inserter(rv, rv.begin()));
        return rv;
    }
}

VcfGenotypeMatcher::VcfGenotypeMatcher(
          std::vector<std::string> const& streamNames
        , std::vector<std::string> const& sampleNames
        , std::string const& exactFieldName
        , std::string const& partialFieldName
        , std::vector<Vcf::FilterType> const& filterTypes
        , EntryOutput& entryOutput
        )
    : numFiles_(streamNames.size())
    , numSamples_(sampleNames.size())
    , exactFieldName_(exactFieldName)
    , partialFieldName_(partialFieldName)
    , streamNames_(streamNames)
    , sampleNames_(sampleNames)
    , entryOutput_(entryOutput)
    , filterTypes_(filterTypes)
    , gtDicts_(numSamples_)
    , sampleCounters_(numSamples_)
{
}

void VcfGenotypeMatcher::collectEntry(size_t entryIdx) {
    Vcf::Entry const& entry = *entries_[entryIdx];
    size_t fileIdx = entry.header().sourceIndex();
    auto rawvs = RawVariant::processEntry(entry);
    auto const& sampleData = entry.sampleData();

    SampleGenotypes sampleGenotypes(numSamples_);

    for (uint32_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        uint32_t sampleIdx = entry.header().sampleIndex(sampleNames_[rawSampleIdx]);
        bool filtered = entry.isFiltered() || sampleData.isSampleFiltered(sampleIdx);
        if (shouldSkip(fileIdx, filtered))
            continue;

        GenotypeCall const& call = sampleData.genotype(sampleIdx);
        if (call == GenotypeCall::Null)
            continue;

        // FIXME: try to copy the RawVariants less
        RawVariant::Vector gtvec;
        for (auto idx = call.indices().begin(); idx != call.indices().end(); ++idx) {
            if (*idx != Vcf::GenotypeIndex::Null && idx->value > 0) {
                auto const& allele = rawvs[idx->value - 1];
                gtvec.push_back(new RawVariant(allele));
            }
        }

        gtvec.sort();
        gtDicts_[rawSampleIdx].add(gtvec, fileIdx);
        sampleGenotypes[rawSampleIdx] = std::move(gtvec);
    }

    entryGenotypes_.push_back(std::move(sampleGenotypes));
}

void VcfGenotypeMatcher::updateCounts() {
    for (size_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        // alleleMatches: allele -> (fileIndex -> count)
        auto alleleMatches = gtDicts_[rawSampleIdx].copyPartialMatches();
        for (auto al = alleleMatches.begin(); al != alleleMatches.end(); ++al) {
            auto& locationCounts = al->second;
            while (!locationCounts.empty()) {
                std::set<size_t> files;
                for (auto lc = locationCounts.begin(); lc != locationCounts.end(); ) {
                    size_t fileIdx = lc->first;
                    size_t& count = lc->second;
                    if (count > 0) {
                        files.insert(fileIdx);
                        --count;
                    }

                    if (count == 0)
                        lc = locationCounts.erase(lc);
                    else
                        ++lc;
                }
                ++sampleCounters_[rawSampleIdx][files];
            }
        }
    }
}

auto VcfGenotypeMatcher::partialMatchingFiles(
      GenotypeDict const& dict
    , Vcf::RawVariant::Vector const& genotype
    )
    -> boost::container::flat_set<FileIndex>
{
    flat_set<size_t> partials;
    boost::unordered_set<RawVariant> seen;
    for (auto ai = genotype.begin(); ai != genotype.end(); ++ai) {
        auto const& allele = *ai;

        auto inserted = seen.insert(allele);
        if (!inserted.second)
            continue; // already processed this allele

        auto const& xsec = dict.allMatches(allele);
        for (auto j = xsec.begin(); j != xsec.end(); ++j) {
            size_t fileIdx = j->first;
            partials.insert(fileIdx);
        }
    }

    return partials;
}


void VcfGenotypeMatcher::annotateEntry(size_t entryIdx) {
    Entry& entry = *entries_[entryIdx];
    auto exactType = getType(entry.header(), exactFieldName_);
    auto partialType = getType(entry.header(), partialFieldName_);

    auto const& sampleGenotypes = entryGenotypes_[entryIdx];
    auto& sampleData = entry.sampleData();

    for (uint32_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        auto const& dict = gtDicts_[rawSampleIdx];
        auto const& genotype = sampleGenotypes[rawSampleIdx];

        flat_set<size_t> partials = partialMatchingFiles(dict, genotype);
        auto const& exactMatches = dict.exactMatches(sampleGenotypes[rawSampleIdx]);

        std::vector<Vcf::CustomValue::ValueType> exactValues(numFiles_, int64_t{0});
        std::vector<Vcf::CustomValue::ValueType> partialValues(numFiles_, int64_t{0});

        partials = difference_(partials, exactMatches);
        setValues(exactValues, exactMatches);
        setValues(partialValues, partials);

        uint32_t sampleIdx = entry.header().sampleIndex(sampleNames_[rawSampleIdx]);
        sampleData.setSampleField(sampleIdx, Vcf::CustomValue(exactType, std::move(exactValues)));
        sampleData.setSampleField(sampleIdx, Vcf::CustomValue(partialType, std::move(partialValues)));
    }
}

void VcfGenotypeMatcher::operator()(EntryList&& entries) {
    entries_.swap(entries);

    for (size_t i = 0; i < entries_.size(); ++i)
        collectEntry(i);

    assert(entryGenotypes_.size() == entries_.size());

    for (size_t i = 0; i < entries_.size(); ++i)
        annotateEntry(i);

    updateCounts();
    writeEntries();
    reset();
}

void VcfGenotypeMatcher::reset() {
    for (auto di = gtDicts_.begin(); di != gtDicts_.end(); ++di)
        di->clear();

    entryGenotypes_.clear();
    entries_.clear();
}

void VcfGenotypeMatcher::writeEntries() const {
    for (auto ei = entries_.begin(); ei != entries_.end(); ++ei) {
        entryOutput_(**ei);
    }
}

std::set<size_t> intToSet(uint32_t x) {
    std::set<size_t> rv;
    for (size_t i = 0; i < 8 * sizeof(x); ++i) {
        uint32_t bit = 1 << i;
        if (x & bit)
            rv.insert(i);
    }
    return rv;
}

void VcfGenotypeMatcher::reportCounts(std::ostream& os) const {
    os << streamJoin(reversed(streamNames_)).delimiter("\t")
        << "\t" << streamJoin(sampleNames_).delimiter("\t") << "\n";

    uint32_t numRows = (1 << numFiles_) - 1;
    std::vector<char> zeros(numFiles_, '0');

    for (uint32_t row = 1; row <= numRows; ++row) {
        std::string indicator = integerToBinary(row);
        indicator = indicator.substr(indicator.size() - numFiles_);

        os << streamJoin(indicator).delimiter("\t");

        for (size_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
            auto const& counts = sampleCounters_[rawSampleIdx];
            auto indices = intToSet(row);
            auto found = counts.find(indices);
            size_t count = 0;
            if (found != counts.end())
                count = found->second;
            os << "\t" << count;
        }
        os << "\n";
    }
}

bool VcfGenotypeMatcher::shouldSkip(size_t streamIdx, bool isFiltered) const {
    // if the filter status doesn't agree with the command line input, skip it (true)
    // otherwise return false
    FilterType status = isFiltered ? Vcf::eFILTERED : Vcf::eUNFILTERED;
    return (int(filterTypes_[streamIdx]) & int(status)) == 0;
}
