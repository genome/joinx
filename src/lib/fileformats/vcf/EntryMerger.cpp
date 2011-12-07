#include "EntryMerger.hpp"

#include "CustomType.hpp"
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

BEGIN_NAMESPACE(Vcf)

namespace {
    bool referenceSizeLessThan(const Entry& a, const Entry& b) {
        return a.ref().size() < b.ref().size();
    }
}

size_t EntryMerger::addAllele(const string& allele) {
    auto inserted = _alleleMap.insert(make_pair(allele, _alleleIdx));
    if (inserted.second)
        ++_alleleIdx;
    return inserted.first->second;
}

EntryMerger::EntryMerger(const MergeStrategy& mergeStrategy, const Header* mergedHeader, const Entry* begin, const Entry* end)
    : _mergeStrategy(mergeStrategy)
    , _mergedHeader(mergedHeader)
    , _begin(begin)
    , _end(end)
    , _refEntry(max_element(begin, end, referenceSizeLessThan))
    , _qual(Entry::MISSING_QUALITY)
    , _alleleIdx(0)
{
    _newGTIndices.resize(_end - _begin);

    if (end-begin == 1)
        _qual = begin->qual();

    for (const Entry* e = begin; e != end; ++e) {
        size_t idx = e-begin;

        if (e > begin && (e->chrom() != begin->chrom() || e->pos() != begin->pos())) {
            throw runtime_error(
                str(format("Attempted to merge VCF entries with different position:\n%1%\nand\n%2%")
                    %begin->toString() %e->toString()));
        }

        string::size_type refLen = e->ref().size();
        if (e->ref().compare(0, refLen, ref(), 0, refLen) != 0) {
            throw runtime_error(str(format(
                "Attempted to merge VCF entries with incompatible ref entries:\n%1%\nand\n%2%")
                %_refEntry->toString() %e->toString()));
        }

        // merge identifiers
        const set<string>& idents = e->identifiers();
        copy(idents.begin(), idents.end(), inserter(_identifiers, _identifiers.begin()));

        // Merge alleles
        const vector<string>& alleles = e->alt();
        for (auto alt = alleles.begin(); alt != alleles.end(); ++alt) {
            if (refLen != ref().size()) {
                string allele = *alt;
                allele += ref().substr(refLen);
                _newGTIndices[idx].push_back(addAllele(allele));
            } else {
                _newGTIndices[idx].push_back(addAllele(*alt));
            }

        }

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
    return _refEntry->ref();
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
        std::vector<CustomType const*>& format,
        std::map<uint32_t, std::vector<CustomValue> >& sampleData) const
{
    // set alt alleles
    alt.resize(_alleleMap.size());
    for (auto i = _alleleMap.begin(); i != _alleleMap.end(); ++i)
        alt[i->second] = i->first;

    // build list of all format fields
    GenotypeFormatter genotypeFormatter(_mergedHeader, alt);
    set<string> seen;
    for (const Entry* e = _begin; e != _end; ++e) {
        const vector<CustomType const*>& gtFormat = e->formatDescription();
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

    for (const Entry* e = _begin; e != _end; ++e) {
        Entry::SampleData const& samples = e->sampleData();
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
                auto inserted = sampleData.insert(make_pair(mergedIdx, vector<CustomValue>()));
                if (inserted.second || inserted.first->second.empty()) {
                    inserted.first->second = genotypeFormatter.process(format, e, sampleIdx, _newGTIndices[idx]);
                } else if (_mergeStrategy.mergeSamples()) {
                    bool fromPrimaryStream = e->header().sourceIndex() == _mergeStrategy.primarySampleStreamIndex();
                    genotypeFormatter.merge(fromPrimaryStream, inserted.first->second, format, e, sampleIdx, _newGTIndices[idx]);
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
}

const Header* EntryMerger::mergedHeader() const {
    return _mergedHeader;
}

END_NAMESPACE(Vcf)
