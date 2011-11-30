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
        const vector<string>& idents = e->identifiers();
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
        const vector<string>& filters = e->failedFilters();
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

const set<string>& EntryMerger::identifiers() const {
    return _identifiers;
}

const string& EntryMerger::ref() const {
    return _refEntry->ref();
}

const set<string>& EntryMerger::failedFilters() const {
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
        std::vector<std::string>& format,
        std::vector< std::vector<CustomValue> >& sampleData) const
{
    // set alt alleles
    alt.resize(_alleleMap.size());
    for (auto i = _alleleMap.begin(); i != _alleleMap.end(); ++i)
        alt[i->second] = i->first;

    // build list of all format fields
    sampleData.resize(_mergedHeader->sampleNames().size());
    GenotypeFormatter genotypeFormatter(_mergedHeader, alt);
    set<string> seen;
    for (const Entry* e = _begin; e != _end; ++e) {
        const vector<string>& gtFormat = e->formatDescription();
        for (auto i = gtFormat.begin(); i != gtFormat.end(); ++i) {
            auto inserted = seen.insert(*i);
            if (inserted.second) {
                // GT field must always come first as per VCF4.1 spec
                if (*i == "GT" && !format.empty()) {
                    format.push_back(format[0]);
                    format[0] = *i;
                } else {
                    format.push_back(*i);
                }
            }
        }
    }

    for (const Entry* e = _begin; e != _end; ++e) {
        const vector< vector<CustomValue> >& samples = e->sampleData();
        try {
            for (uint32_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
                if (samples[sampleIdx].empty())
                    continue;
                const string& sampleName = e->header().sampleNames()[sampleIdx];
                uint32_t mergedIdx = _mergedHeader->sampleIndex(sampleName);
                size_t idx = e - _begin;
                if (!sampleData[mergedIdx].empty() && _mergeStrategy.mergeSamples() && !sampleData[mergedIdx].empty()) {
                    bool fromPrimaryStream = e->header().sourceIndex() == _mergeStrategy.primarySampleStreamIndex();
                    genotypeFormatter.merge(fromPrimaryStream, sampleData[mergedIdx], format, e, sampleIdx, _newGTIndices[idx]);
                } else {
                    sampleData[mergedIdx] = genotypeFormatter.process(format, e, sampleIdx, _newGTIndices[idx]);
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
