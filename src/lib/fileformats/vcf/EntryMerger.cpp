#include "EntryMerger.hpp"

#include "CustomValue.hpp"
#include "Entry.hpp"
#include "GenotypeFormatter.hpp"
#include "Header.hpp"
#include "MergeStrategy.hpp"
#include <boost/format.hpp>
#include <algorithm>
#include <iterator>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

EntryMerger::EntryMerger(const MergeStrategy& mergeStrategy, const Header* mergedHeader, const Entry* begin, const Entry* end)
    : _mergeStrategy(mergeStrategy)
    , _mergedHeader(mergedHeader)
    , _begin(begin)
    , _end(end)
{
    uint32_t alleleIdx = 0;
    uint32_t qualCount(0);
    for (const Entry* e = begin; e != end; ++e) {
        if (e->qual() != Entry::MISSING_QUALITY) {
            ++qualCount;
            _qual = e->qual();
        }

        if (e > begin &&
            (e->chrom() != begin->chrom() || e->pos() != begin->pos() || e->ref() != begin->ref())) {
            throw runtime_error(
                str(format("Attempted to merge VCF entries with different position or reference: %1% and %2%")
                    %begin->toString() %e->toString()));
        }

        // merge identifiers
        const vector<string>& idents = e->identifiers();
        copy(idents.begin(), idents.end(), inserter(_identifiers, _identifiers.begin()));

        // Merge alleles
        const vector<string>& alleles = e->alt();
        for (auto alt = alleles.begin(); alt != alleles.end(); ++alt) {
            auto inserted = _alleleMap.insert(make_pair(*alt, alleleIdx));
            if (inserted.second)
                ++alleleIdx;
        }

        // Merge filters
        const vector<string>& filters = e->failedFilters();
        copy(filters.begin(), filters.end(), inserter(_filters, _filters.begin()));

        const vector<string>& samples = e->header().sampleNames();
        for (auto i = samples.begin(); i != samples.end(); ++i) {
            auto inserted = _sampleNames.insert(*i);
            if (!inserted.second)
                throw runtime_error(str(format("Duplicate sample name '%1%' in %2%") %*i %e->toString()));
        }

        // Build set of all info fields present, validating as we go
        const CustomValueMap& info = e->info();
        for (auto i = info.begin(); i != info.end(); ++i) {
            _info.insert(i->first);
            if (!_mergedHeader->infoType(i->first)) {
                throw runtime_error(str(format("Invalid info field '%1%' while merging vcf entries in %2%") %i->first %e->toString()));
            }
        }
    }

    // If there was only a single qual value, we will use it. Otherwise, it's not clear how to merge them
    if (qualCount > 1)
        _qual = Entry::MISSING_QUALITY;
}

const string& EntryMerger::chrom() const {
    return _begin->chrom();
}

uint64_t EntryMerger::pos() const {
    return _begin->pos();
}

const set<string>& EntryMerger::identifiers() const {
    return _identifiers;
}

const string& EntryMerger::ref() const {
    return _begin->ref();
}

const EntryMerger::AlleleMap& EntryMerger::alleleMap() const {
    return _alleleMap;
}

const set<string>& EntryMerger::failedFilters() const {
    return _filters;
}

double EntryMerger::qual() const {
    // TODO: figure out if we want to do some kind of actual merging here
    return _qual;
}

void EntryMerger::setInfo(CustomValueMap& info) const {
    for (auto i = _info.begin(); i != _info.end(); ++i) {
        CustomValue v = _mergeStrategy.mergeInfo(*i, _begin, _end);
        info.insert(make_pair(*i, v));
    }
}

void EntryMerger::setGenotypeData(
        std::vector<std::string>& format,
        std::vector< std::vector<CustomValue> >& genotypeData) const
{
    genotypeData.resize(_mergedHeader->sampleNames().size());
    GenotypeFormatter genotypeFormatter(_mergedHeader, _alleleMap);
    set<string> seen;
    for (const Entry* e = _begin; e != _end; ++e) {
        const vector<string>& gtFormat = e->formatDescription();
        for (auto i = gtFormat.begin(); i != gtFormat.end(); ++i) {
            auto inserted = seen.insert(*i);
            if (inserted.second)
                format.push_back(*i);
        }
    }

    for (const Entry* e = _begin; e != _end; ++e) {
        const vector< vector<CustomValue> >& samples = e->genotypeData();
        for (uint32_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
            if (samples[sampleIdx].empty())
                continue;
            const string& sampleName = e->header().sampleNames()[sampleIdx];
            uint32_t mergedIdx = _mergedHeader->sampleIndex(sampleName);
            genotypeData[mergedIdx] = genotypeFormatter.process(format, e, sampleIdx);
        }
    }
}

const Header* EntryMerger::mergedHeader() const {
    return _mergedHeader;
}

VCF_NAMESPACE_END
