#include "VcfGenotypeMatcher.hpp"

#include "common/Integer.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Header.hpp"
#include "io/StreamJoin.hpp"

#include <boost/format.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <stdexcept>

using namespace Vcf;
using boost::container::flat_set;
using boost::format;


namespace {
    RawVariant const NullAllele(0, ".", ".");

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
    , filterTypes_(filterTypes)
    , entryOutput_(entryOutput)
    , gtDicts_(numSamples_)
    , partialSampleCounters_(numSamples_)
    , exactSampleCounters_(numSamples_)
    , partialMissSampleCounters_(numSamples_)
    , completeMissSampleCounters_(numSamples_)
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
            if (*idx == Vcf::GenotypeIndex::Null) {
                gtvec.push_back(new RawVariant(NullAllele));
            }
            else if (idx->value > 0) {
                auto const& allele = rawvs[idx->value - 1];
                gtvec.push_back(new RawVariant(allele));
            }
        }

        gtvec.sort();
        gtDicts_[rawSampleIdx].add(gtvec, entryIdx);
        sampleGenotypes[rawSampleIdx] = std::move(gtvec);
    }

    entryGenotypes_.push_back(std::move(sampleGenotypes));
}

auto VcfGenotypeMatcher::entryToFileIndex(EntryIndex idx) const -> FileIndex{
    return entries_[idx]->header().sourceIndex();
}

void VcfGenotypeMatcher::updateCounts() {
    for (size_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        std::vector<size_t> partialMisses(entries_.size(), 0u);
        flat_set<EntryIndex> entriesWithMatches;
        // alleleMatches: allele -> (fileIndex -> count)
        auto const& alleleMatches = gtDicts_[rawSampleIdx].partialMatches();
        for (auto al = alleleMatches.begin(); al != alleleMatches.end(); ++al) {
            auto& locationCounts = al->second;
            std::set<EntryIndex> entries;
            std::set<FileIndex> files;
            for (auto lc = locationCounts.begin(); lc != locationCounts.end(); ++lc) {
                size_t entryIdx = lc->first;
                entries.insert(entryIdx);
            }
            entryToFileIndices(entries, files);

            if (entries.size() == 1)
                ++partialMisses[*entries.begin()];
            else
                entriesWithMatches.insert(entries.begin(), entries.end());

            uint64_t fileBits = setBits<uint64_t>(files);
            ++partialSampleCounters_[rawSampleIdx][fileBits];
        }

        auto const& gtMatches = gtDicts_[rawSampleIdx].exactMatches();
        for (auto gt = gtMatches.begin(); gt != gtMatches.end(); ++gt) {
            std::set<FileIndex> files;
            std::set<EntryIndex> entries;
            auto const& locations = gt->second;
            for (auto loc = locations.begin(); loc != locations.end(); ++loc) {
                size_t entryIdx = *loc;
                entries.insert(entryIdx);
            }
            entryToFileIndices(entries, files);

            if (locations.size() > 1)
                entriesWithMatches.insert(entries.begin(), entries.end());

            uint64_t fileBits = setBits<uint64_t>(files);
            ++exactSampleCounters_[rawSampleIdx][fileBits];
        }

        for (size_t entryIdx = 0; entryIdx < partialMisses.size(); ++entryIdx) {
            if (partialMisses[entryIdx] > 0) {
                size_t fileIdx = entries_[entryIdx]->header().sourceIndex();
                uint64_t idx = 1 << fileIdx;
                if (entriesWithMatches.count(entryIdx))
                    partialMissSampleCounters_[rawSampleIdx][idx] += partialMisses[entryIdx];
                else
                    completeMissSampleCounters_[rawSampleIdx][idx] += partialMisses[entryIdx];
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
            size_t entryIdx = j->first;
            size_t fileIdx = entries_[entryIdx]->header().sourceIndex();
            partials.insert(fileIdx);
        }
    }

    return partials;
}

bool VcfGenotypeMatcher::hasNullAllele(EntryIndex entryIdx, size_t sampleIdx) const {
    auto const& genotype = entryGenotypes_[entryIdx][sampleIdx];
    return std::find(genotype.begin(), genotype.end(), NullAllele) != genotype.end();
}

void VcfGenotypeMatcher::annotateEntry(size_t entryIdx) {
    Entry& entry = *entries_[entryIdx];
    FileIndex fileIdx = entryToFileIndex(entryIdx);
    auto exactType = getType(entry.header(), exactFieldName_);
    auto partialType = getType(entry.header(), partialFieldName_);

    auto const& sampleGenotypes = entryGenotypes_[entryIdx];
    auto& sampleData = entry.sampleData();

    for (uint32_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        auto const& dict = gtDicts_[rawSampleIdx];
        auto const& genotype = sampleGenotypes[rawSampleIdx];

        flat_set<size_t> partials = partialMatchingFiles(dict, genotype);
        auto exactMatchEntries = dict.exactMatches(sampleGenotypes[rawSampleIdx]);
        // Don't allow genotypes with null alleles to be exact matches with
        // anything other than themselves
        if (hasNullAllele(entryIdx, rawSampleIdx)) {
            exactMatchEntries.clear();
            exactMatchEntries.insert(fileIdx);
        }

        flat_set<size_t> exactMatches;
        for (auto ei = exactMatchEntries.begin(); ei != exactMatchEntries.end(); ++ei) {
            exactMatches.insert(entries_[*ei]->header().sourceIndex());
        }

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

void VcfGenotypeMatcher::printCounts_(
        std::ostream& os,
        std::vector<SampleCounter> const& counts
        , std::string const& type
        ) const
{
    uint32_t numRows = (1 << numFiles_) - 1;
    std::vector<char> zeros(numFiles_, '0');

    for (uint32_t row = 1; row <= numRows; ++row) {
        std::string indicator = integerToBinary(row);
        indicator = indicator.substr(indicator.size() - numFiles_);

        os << streamJoin(indicator).delimiter("\t") << "\t" << type;
        for (size_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
            auto const& sampleCounts = counts[rawSampleIdx];
            auto found = sampleCounts.find(row);
            size_t count = 0;
            if (found != sampleCounts.end())
                count = found->second;
            os << "\t" << count;
        }
        os << "\n";
    }
}

void VcfGenotypeMatcher::reportCounts(std::ostream& os) const {
    os << streamJoin(reversed(streamNames_)).delimiter("\t")
        << "\t" << "match_type"
        << "\t" << streamJoin(sampleNames_).delimiter("\t")
        << "\n";

    printCounts_(os, partialSampleCounters_, "partial_hit");
    printCounts_(os, exactSampleCounters_, "exact_hit");
    printCounts_(os, partialMissSampleCounters_, "partial_miss");
    printCounts_(os, completeMissSampleCounters_, "complete_miss");
}

bool VcfGenotypeMatcher::shouldSkip(size_t streamIdx, bool isFiltered) const {
    // if the filter status doesn't agree with the command line input, skip it (true)
    // otherwise return false
    FilterType status = isFiltered ? Vcf::eFILTERED : Vcf::eUNFILTERED;
    return (int(filterTypes_[streamIdx]) & int(status)) == 0;
}
