#include "VcfGenotypeMatcher.hpp"

#include "fileformats/vcf/Header.hpp"
#include "io/StreamJoin.hpp"


#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

using namespace Vcf;
using boost::container::flat_set;


namespace {
    template<typename Container>
    Container intersect_(Container const& s1, Container const& s2) {
        Container rv;
        std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
            std::inserter(rv, rv.begin()));
        return rv;
    }

    template<typename Container>
    Container difference_(Container const& s1, Container const& s2) {
        Container rv;
        std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
            std::inserter(rv, rv.begin()));
        return rv;
    }
}

void GenotypeDictionary::add(Vcf::RawVariant::Vector const& gt, size_t entryIdx) {
    exactMap_[gt].insert(entryIdx);
    for (auto x = gt.begin(); x != gt.end(); ++x) {
        ++partialMap_[*x][entryIdx];
    }
}

auto GenotypeDictionary::allMatches(Vcf::RawVariant const& gt) const -> LocationCounts const* {
    auto iter = partialMap_.find(gt);
    if (iter != partialMap_.end()) {
        return &iter->second;
    }
    return 0;
}

auto GenotypeDictionary::exactMatches(Vcf::RawVariant::Vector const& gt) const -> Locations const* {
    auto iter = exactMap_.find(gt);
    if (iter != exactMap_.end()) {
        return &iter->second;
    }
    return 0;
}

void GenotypeDictionary::clear() {
    partialMap_.clear();
    exactMap_.clear();
}


VcfGenotypeMatcher::VcfGenotypeMatcher(uint32_t numFiles, uint32_t numSamples)
    : numFiles_(numFiles)
    , numSamples_(numSamples)
    , gtDicts_(numSamples)
    , sampleCounters_(numSamples)
{
}

void VcfGenotypeMatcher::collectEntry(size_t entryIdx) {
    Vcf::Entry const& entry = *entries_[entryIdx];
    auto rawvs = RawVariant::processEntry(entry);
    auto const& sampleData = entry.sampleData();

    SampleGenotypes sampleGenotypes(numSamples_);

    for (uint32_t sampleIdx = 0; sampleIdx < numSamples_; ++sampleIdx) {
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
        gtDicts_[sampleIdx].add(gtvec, entryIdx);
        sampleGenotypes[sampleIdx] = std::move(gtvec);
    }

    entryGenotypes_.push_back(std::move(sampleGenotypes));
}

void VcfGenotypeMatcher::getCounts() {

    for (size_t sampleIdx = 0; sampleIdx < numSamples_; ++sampleIdx) {
        auto alleleMatches = gtDicts_[sampleIdx].copyPartialMatches();
        for (auto al = alleleMatches.begin(); al != alleleMatches.end(); ++al) {
            auto& locationCounts = al->second;
            std::set<size_t> files;
            for (auto lc = locationCounts.begin(); lc != locationCounts.end(); ) {
                size_t entryIdx = lc->first;
                size_t fileIdx = entries_[entryIdx]->header().sourceIndex();
                size_t& count = lc->second;
                if (count > 0) {
                    files.insert(fileIdx);
                    --count;
                    ++lc;
                }
                else {
                    lc = locationCounts.erase(lc);
                }
            }
            ++sampleCounters_[sampleIdx][files];
        }
    }
}

void VcfGenotypeMatcher::annotateEntry(size_t entryIdx) {
    Entry& entry = *entries_[entryIdx];

    auto const& sampleGenotypes = entryGenotypes_[entryIdx];

    for (uint32_t sampleIdx = 0; sampleIdx < numSamples_; ++sampleIdx) {
        auto const& dict = gtDicts_[sampleIdx];
        auto const& genotype = sampleGenotypes[sampleIdx];

        flat_set<size_t> partials;

        auto const* exactMatches = dict.exactMatches(sampleGenotypes[sampleIdx]);

        // FIXME: don't copy vars; make and use a pointer hasher
        boost::unordered_set<RawVariant> seen;
        for (auto ai = genotype.begin(); ai != genotype.end(); ++ai) {
            auto inserted = seen.insert(*ai);
            if (!inserted.second)
                continue; // already processed this allele

            auto const& allele = *ai;

            auto const* xsec = dict.allMatches(allele);
            if (xsec) {
                for (auto j = xsec->begin(); j != xsec->end(); ++j) {
                    size_t whichEntry = j->first;
                    partials.insert(whichEntry);
                }
            }
        }
        if (exactMatches) {
            partials = difference_(partials, *exactMatches);
        }
    }
}

void VcfGenotypeMatcher::operator()(EntryList&& entries) {
    entries_.swap(entries);

    for (size_t i = 0; i < entries_.size(); ++i)
        collectEntry(i);

    assert(entryGenotypes_.size() == entries_.size());

    for (size_t i = 0; i < entries_.size(); ++i)
        annotateEntry(i);

    getCounts();
    reset();
}

void VcfGenotypeMatcher::reset() {
    for (auto di = gtDicts_.begin(); di != gtDicts_.end(); ++di)
        di->clear();

    entryGenotypes_.clear();
    entries_.clear();
}

void VcfGenotypeMatcher::finalize() {
    for (size_t sampleIdx = 0; sampleIdx < numSamples_; ++sampleIdx) {
        std::cout << "Sample " << sampleIdx << "\n";
        auto const& counts = sampleCounters_[sampleIdx];
        for (auto ci = counts.begin(); ci != counts.end(); ++ci) {
            auto const& fileSet = ci->first;
            size_t count = ci->second;
            std::cout << streamJoin(fileSet) << ": " << count << "\n";
        }
    }
}
