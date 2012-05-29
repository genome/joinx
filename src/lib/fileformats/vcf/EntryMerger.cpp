#include "EntryMerger.hpp"

#include "ConsensusFilter.hpp"
#include "CustomType.hpp"
#include "CustomValue.hpp"
#include "Entry.hpp"
#include "GenotypeMerger.hpp"
#include "Header.hpp"
#include "MergeStrategy.hpp"
#include "SampleData.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>

using boost::format;
using namespace std;
using namespace std::placeholders;

BEGIN_NAMESPACE(Vcf)

namespace {
    bool isBetter(
        Entry const* e,
        Entry const* currBest,
        size_t sampleIdx,
        MergeStrategy::SamplePriority prio)
    {
        if (currBest == NULL)
            return true;

        uint32_t nsrc = e->header().sourceIndex(); // src index of new entry
        uint32_t csrc = currBest->header().sourceIndex(); // src index of curr

        // is new entry filtered?
        bool nf = e->sampleData().isSampleFiltered(sampleIdx);
        // is currBest filtered?
        bool cf = currBest->sampleData().isSampleFiltered(sampleIdx);


        // if we only care about order or the new entry and the current best have the
        // same filter status sort by order
        if (prio == MergeStrategy::eORDER || nf == cf)
            return nsrc < csrc;

        // at this point, we know nf != cf
        if (prio == MergeStrategy::eFILTERED)
            return nf;
        else if (prio == MergeStrategy::eUNFILTERED)
            return !nf;
        else
            throw logic_error(str(format(
                "Programming error at %1%:%2%: didn't understand sample priority: %3%"
                ) %__FILE__ %__LINE__ %int(prio)));
    }
}

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
    , _sampleCounts(_mergedHeader->sampleCount(), 0ul)
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
    GenotypeMerger genotypeFormatter(_mergedHeader, alt);
    set<string> seen; // keep track of what fields we have already seen
    for (const Entry* e = _begin; e != _end; ++e) {
        const vector<CustomType const*>& gtFormat = e->sampleData().format();
        for (auto i = gtFormat.begin(); i != gtFormat.end(); ++i) {
            // check if we have already seen this field.
            auto inserted = seen.insert((*i)->id());

            // if it was inserted, then it is new
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

    // build a set of all sample indices with data across all entries
    std::set<uint32_t> sampleIndices;
    for (const Entry* e = _begin; e != _end; ++e) {
        SampleData const& samples = e->sampleData();
        for (auto i = samples.begin(); i != samples.end(); ++i)
            sampleIndices.insert(i->first);
    }

    SampleData::MapType sdMap;
    // for each sample index where at least one entry has data...
    for (auto si = sampleIndices.begin(); si != sampleIndices.end(); ++si) {
        uint32_t sampleIdx = *si;
        int primaryEntryIdx = getPrimaryEntryIdx(sampleIdx);

        for (Entry const* e = _begin; e != _end; ++e) {
            bool overridePreviousData = (e-_begin) == primaryEntryIdx;
            SampleData const& samples = e->sampleData();
            try {
                vector<CustomValue> const* values = samples.get(sampleIdx);
                if (!values || values->empty())
                    continue;

                const string& sampleName = e->header().sampleNames()[sampleIdx];
                uint32_t mergedIdx = _mergedHeader->sampleIndex(sampleName);
                size_t idx = e - _begin;

                auto inserted = sdMap.insert(make_pair(mergedIdx, vector<CustomValue>()));
                if (inserted.second || inserted.first->second.empty()) {
                    inserted.first->second = genotypeFormatter.process(format, e, sampleIdx, _alleleMerger.newGt()[idx]);
                    if (!e->sampleData().isSampleFiltered(sampleIdx))
                        ++_sampleCounts[mergedIdx];
                } else if (_mergeStrategy.mergeSamples()) {
                    genotypeFormatter.merge(overridePreviousData, inserted.first->second, format, e, sampleIdx, _alleleMerger.newGt()[idx]);
                    if (!e->sampleData().isSampleFiltered(sampleIdx))
                        ++_sampleCounts[mergedIdx];
                } else {
                    throw runtime_error("Unable to merge conflicting sample data.");
                }
            } catch (const DisjointGenotypesError&) {
                // don't do anything for now
            } catch (...) {
                throw;
            }
        }
    }

    sampleData = SampleData(_mergedHeader, std::move(format), std::move(sdMap));
}

int EntryMerger::getPrimaryEntryIdx(size_t sampleIdx) const {
    Entry const* best = _begin;
    auto prio = _mergeStrategy.samplePriority();
    for (Entry const* e = _begin+1; e != _end; ++e) {
        if (isBetter(e, best, sampleIdx, prio))
            best = e;
    }

    if (best == _end)
        return 0;
    else
        return best - _begin;
}

const Header* EntryMerger::mergedHeader() const {
    return _mergedHeader;
}

std::vector<size_t> const& EntryMerger::sampleCounts() const {
    return _sampleCounts;
}

END_NAMESPACE(Vcf)
