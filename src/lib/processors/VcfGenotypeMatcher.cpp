#include "VcfGenotypeMatcher.hpp"

#include "fileformats/vcf/Header.hpp"
#include "io/StreamJoin.hpp"

#include <boost/container/flat_set.hpp>

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

void GenotypeDictionary::add(Vcf::RawVariant const& gt, size_t count, size_t entryIdx) {
    partialMap_[gt][entryIdx] += count;
}

auto GenotypeDictionary::match(Vcf::RawVariant const& gt) const -> LocationCounts const* {
    auto iter = partialMap_.find(gt);
    if (iter != partialMap_.end()) {
        return &iter->second;
    }
    return 0;
}

void GenotypeDictionary::clear() {
    partialMap_.clear();
}


VcfGenotypeMatcher::VcfGenotypeMatcher(uint32_t numFiles, uint32_t numSamples)
    : numFiles_(numFiles)
    , numSamples_(numSamples)
    , gtDicts_(numSamples)
{
}

void VcfGenotypeMatcher::collectEntry(size_t entryIdx) {
    Vcf::Entry const& entry = *entries_[entryIdx];
    size_t fileIdx = entry.header().sourceIndex();
    auto rawvs = RawVariant::processEntry(entry);
    auto const& sampleData = entry.sampleData();

    SampleAlleleCounts sampleAlleleCounts(numSamples_);

    for (uint32_t sampleIdx = 0; sampleIdx < numSamples_; ++sampleIdx) {
        GenotypeCall const& call = sampleData.genotype(sampleIdx);
        if (call == GenotypeCall::Null)
            continue;

        // FIXME: avoid copying the RawVariant here
        AlleleCounts alleleCounts;
        for (auto idx = call.indices().begin(); idx != call.indices().end(); ++idx) {
            if (*idx != Vcf::GenotypeIndex::Null && idx->value > 0)
                ++alleleCounts[rawvs[idx->value - 1]];
        }

        for (auto ac = alleleCounts.begin(); ac != alleleCounts.end(); ++ac) {
            gtDicts_[sampleIdx].add(ac->first, ac->second, fileIdx);
        }

        sampleAlleleCounts[sampleIdx] = std::move(alleleCounts);
    }
    entryAlleleCounts_.push_back(std::move(sampleAlleleCounts));
}

void VcfGenotypeMatcher::getCounts() {
}

void VcfGenotypeMatcher::annotateEntry(size_t entryIdx) {
    Entry& entry = *entries_[entryIdx];

    size_t fileIdx = entry.header().sourceIndex();
    std::cout << "SOURCE: " << fileIdx << "\n----\n" << entry << "\n";

    auto const& sampleAlleleCounts = entryAlleleCounts_[entryIdx];
    for (uint32_t sampleIdx = 0; sampleIdx < numSamples_; ++sampleIdx) {
        auto const& alleleCounts = sampleAlleleCounts[sampleIdx];

        flat_set<size_t> exactCandidates;
        for (size_t i = 0; i < numFiles_; ++i)
            exactCandidates.insert(i);

        flat_set<size_t> partials;

        for (auto ac = alleleCounts.begin(); ac != alleleCounts.end(); ++ac) {
            auto const& allele = ac->first;
            size_t localAlleleCount = ac->second;
            flat_set<size_t> nextExactCandidates;

            auto const* xsec = gtDicts_[sampleIdx].match(allele);
            std::cout << allele << "\tXSEC:";
            if (xsec) {
                for (auto j = xsec->begin(); j != xsec->end(); ++j) {
                    size_t whichEntry = j->first;
                    size_t otherAlleleCount = j->second;
                    if (otherAlleleCount == localAlleleCount) {
                        nextExactCandidates.insert(whichEntry);
                    }
                    else {
                        partials.insert(whichEntry);
                    }

                    std::cout << j->first << "(" << j->second << "), ";
                }
            }
            else {
                std::cout << "NONE";
            }


            // FIXME: do exact check, this is only checking that this GT is a
            // subset of the ones in exactCandidates
            // It should always work when the number of alleles is the same
            exactCandidates = intersect_(exactCandidates, nextExactCandidates);

            std::cout << "\n";
        }
        partials = difference_(partials, exactCandidates);

        std::cout << "\tExact candidates: " << streamJoin(exactCandidates) << "\n";
        std::cout << "\t Partial matches: " << streamJoin(partials) << "\n";
    }

    std::cout << "\n----\n";
}

void VcfGenotypeMatcher::operator()(EntryList&& entries) {
    entries_.swap(entries);

    for (size_t i = 0; i < entries_.size(); ++i)
        collectEntry(i);

    assert(entryAlleleCounts_.size() == entries_.size());

    for (size_t i = 0; i < entries_.size(); ++i)
        annotateEntry(i);

    reset();
}

void VcfGenotypeMatcher::reset() {
    for (auto di = gtDicts_.begin(); di != gtDicts_.end(); ++di)
        di->clear();

    entryAlleleCounts_.clear();
    entries_.clear();
}
