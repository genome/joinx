#include "EntryMerger.hpp"

#include "CustomType.hpp"
#include "CustomValue.hpp"
#include "Entry.hpp"
#include "GenotypeFormatter.hpp"
#include "Header.hpp"
#include "MergeStrategy.hpp"
#include "SampleData.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <cstring>
#include <iterator>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)

bool EntryMerger::canMerge(Entry const& a, Entry const& b) {
    int rv = strverscmp(a.chrom().c_str(), b.chrom().c_str());
    if (rv != 0)
        return false;

    // to handle identical and adjacent insertions
    if (a.start() == a.stop() && b.start() == b.stop() 
        && (b.start() - a.start() <= 1))
    {
        return true;
    }

    if (a.stop() <= b.start() || b.stop() <= a.start())
        return false;

    return true;
}

EntryMerger::EntryMerger(
        MergeStrategy const& mergeStrategy,
        Header const* mergedHeader,
        Entry const* begin,
        Entry const* end
        )
    : _alleleMerger(begin, end)
    , _mergeStrategy(mergeStrategy)
    , _mergedHeader(mergedHeader)
    , _begin(begin)
    , _end(end)
    , _qual(Entry::MISSING_QUALITY)
{
    if (!_alleleMerger.merged())
        return;

    if (end-begin == 1)
        _qual = begin->qual();

    for (const Entry* e = begin; e != end; ++e) {
        if (e > begin && !canMerge(*e, *(e-1))) {
            throw runtime_error(
                str(format("Attempted to merge VCF entries with non-overlapping position:\n%1%\nand\n%2%")
                    %(e-1)->toString() %e->toString()));
        }

        // merge identifiers
        const set<string>& idents = e->identifiers();
        copy(idents.begin(), idents.end(), inserter(_identifiers, _identifiers.begin()));

        // Merge filters
        const set<string>& filters = e->failedFilters();
        copy(filters.begin(), filters.end(), inserter(_filters, _filters.begin()));

        const vector<string>& samples = e->header().sampleNames();
        for (auto i = samples.begin(); i != samples.end(); ++i) {
            auto inserted = _sampleNames.insert(*i);
            if (!inserted.second && !_mergeStrategy.mergeSamples())
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
    if (mergeStrategy.clearFilters())
        _filters.clear();
    else if (_filters.size() > 1)
        _filters.erase("PASS");
}

bool EntryMerger::merged() const {
    return _alleleMerger.merged();
}

size_t EntryMerger::entryCount() const {
    return _end-_begin;
}

Entry const* EntryMerger::entries() const {
    return _begin;
}

const string& EntryMerger::chrom() const {
    return _begin->chrom();
}

uint64_t EntryMerger::pos() const {
    return _begin->pos();
}

set<string>& EntryMerger::identifiers() {
    return _identifiers;
}

const string& EntryMerger::ref() const {
    return _alleleMerger.ref();
}

set<string>& EntryMerger::failedFilters() {
    return _filters;
}

double EntryMerger::qual() const {
    // TODO: figure out if we want to do some kind of actual merging here
    return _qual;
}

void EntryMerger::setInfo(CustomValueMap& info) const {
    try {
        for (auto i = _info.begin(); i != _info.end(); ++i) {
            CustomValue v = _mergeStrategy.mergeInfo(*i, _begin, _end);
            if (!v.empty())
                info.insert(make_pair(*i, v));
        }
    } catch (const exception& e) {
        throw runtime_error(str(format(
            "Error while merging INFO entries at position %1%,%2%: %3%"
            ) %_begin->chrom() %_begin->pos() %e.what()));
    }
}

void EntryMerger::setAltAndGenotypeData(
        std::vector<std::string>& alt,
        SampleData& sampleData) const
{
    // set alt alleles
    alt = _alleleMerger.mergedAlt();

    // build list of all format fields
    SampleData::FormatType format;
    GenotypeFormatter genotypeFormatter(_mergedHeader, alt);
    set<string> seen;
    for (const Entry* e = _begin; e != _end; ++e) {
        const vector<CustomType const*>& gtFormat = e->sampleData().format();
        for (auto i = gtFormat.begin(); i != gtFormat.end(); ++i) {
            auto inserted = seen.insert((*i)->id());
            if (inserted.second) {
                // GT field must always come first as per VCF4.1 spec
                if ((*i)->id() == "GT" && !format.empty()) {
                    format.push_back(format[0]);
                    format[0] = *i;
                } else {
                    format.push_back(*i);
                }
            }
        }
    }

    SampleData::MapType sdMap;
    for (const Entry* e = _begin; e != _end; ++e) {
        SampleData const& samples = e->sampleData();
        try {
            for (auto i = samples.begin(); i != samples.end(); ++i) {
                auto const& sampleIdx = i->first;
                vector<CustomValue> const& values = i->second;

                if (values.empty())
                    continue;

                const string& sampleName = e->header().sampleNames()[sampleIdx];
                uint32_t mergedIdx = _mergedHeader->sampleIndex(sampleName);
                size_t idx = e - _begin;

                // TODO: try to eliminate double map lookup here
                auto inserted = sdMap.insert(make_pair(mergedIdx, vector<CustomValue>()));
                if (inserted.second || inserted.first->second.empty()) {
                    inserted.first->second = genotypeFormatter.process(format, e, sampleIdx, _alleleMerger.newGt()[idx]);
                } else if (_mergeStrategy.mergeSamples()) {
                    bool fromPrimaryStream = e->header().sourceIndex() == _mergeStrategy.primarySampleStreamIndex();
                    genotypeFormatter.merge(fromPrimaryStream, inserted.first->second, format, e, sampleIdx, _alleleMerger.newGt()[idx]);
                } else {
                    throw runtime_error("Unable to merge conflicting sample data.");
                }
            }
        } catch (const DisjointGenotypesError&) {
            throw;
        } catch (const exception& ex) {
            throw runtime_error(str(boost::format(
                "Failed while merging genotype data for entry:\n%1%\nError: %2%")
                %e->toString() %ex.what()));
        }
    }

    sampleData = SampleData(_mergedHeader, std::move(format), std::move(sdMap));
}

const Header* EntryMerger::mergedHeader() const {
    return _mergedHeader;
}

END_NAMESPACE(Vcf)
