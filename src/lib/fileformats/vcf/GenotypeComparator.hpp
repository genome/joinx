#pragma once

#include "Entry.hpp"
#include "GenotypeCall.hpp"
#include "Header.hpp"
#include "RawVariant.hpp"
#include "common/Region.hpp"
#include "common/cstdint.hpp"
#include "common/namespaces.hpp"
#include "io/StreamJoin.hpp"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

inline
size_t hash_value(RawVariant::Vector const& vs) {
    return boost::hash_range(vs.begin(), vs.end());
}

enum FilterType {
    eUNFILTERED = 1,
    eFILTERED   = 2,
    eBOTH       = 3
};

template<typename OutputWriter>
class GenotypeComparator {
public:
    template<typename HeaderVector>
    GenotypeComparator(
            std::vector<std::string> const& sampleNames,
            HeaderVector const& headers,
            std::vector<FilterType> const& filterTypes,
            size_t numStreams,
            OutputWriter& outputWriter
            )
        : sampleNames_(sampleNames)
        , sampleIndices_(numStreams)
        , filterTypes_(filterTypes)
        , numStreams_(numStreams)
        , entries_(numStreams)
        , out_(outputWriter)
        , final_(false)
        , gtmap_(sampleNames.size())
    {
        for (auto name = sampleNames_.begin(); name != sampleNames_.end(); ++name) {
            for (size_t i = 0; i < headers.size(); ++i) {
                auto const& hdr = headers[i];
                sampleIndices_[i].push_back(hdr->sampleIndex(*name));
            }
        }
    }

    ~GenotypeComparator() {
        finalize();
    }

    void push(Entry* e) {
        if (final_) {
            throw std::runtime_error("Attempted to push entry onto finalized GenotypeComparator");
        }

        if (sequence_.empty()) {
            sequence_ = e->chrom();
            region_.begin = e->start();
        }
        else if (!want(e)) {
            process();
            region_.begin = e->start();
            if (sequence_ != e->chrom()) {
                sequence_ = e->chrom();
            }
        }

        size_t streamIdx = e->header().sourceIndex();
        entries_[streamIdx].push_back(e);
        region_.end = std::max(region_.end, e->stop());
    }

    void finalize() {
        if (!final_) {
            process();
        }
        final_ = true;
    }

private:
    bool want(Entry const* e) const {
        Region entryRegion = {e->start(), e->stop()};
        return sequence_ == e->chrom() && region_.overlap(entryRegion) > 0;
    }

    bool shouldSkip(size_t streamIdx, bool isFiltered) const {
        // if the filter status doesn't agree with the command line input, skip it (true)
        // otherwise return false
        FilterType status = isFiltered ? eFILTERED : eUNFILTERED;
        return (int(filterTypes_[streamIdx]) & int(status)) == 0;
    }

    void processEntries(size_t streamIdx) {
        auto const& ents = entries_[streamIdx];
        for (auto e = ents.begin(); e != ents.end(); ++e) {
            bool siteFiltered = e->isFiltered();
            if (siteFiltered && ((filterTypes_[streamIdx] & eFILTERED) == 0)) {
                /* this for whole line/row FAIL (anything other than PASS or '.') means whole thing fails,
                 * but PASS doesn't mean the whole thing passes, only certain parts */
                continue;
            }

            auto const& sd = e->sampleData();
            RawVariant::Vector rawvs = RawVariant::processEntry(*e);
            if (rawvs.empty()) {
                continue;
            }

            for (size_t sampleIdx = 0; sampleIdx < sampleNames_.size(); ++sampleIdx) {
                RawVariant::Vector alleles;
                bool sampleFiltered = sd.isSampleFiltered(sampleIdx);
                if (shouldSkip(streamIdx, siteFiltered || sampleFiltered)) {
                    continue;
                }

                GenotypeCall const& call = sd.genotype(sampleIndices_[streamIdx][sampleIdx]);
                bool hasRef = false;
                for (auto idx = call.indices().begin(); idx != call.indices().end(); ++idx) {
                    if (*idx > 0) {
                        alleles.push_back(new RawVariant(rawvs[*idx - 1]));
                    }
                    else {
                        hasRef = true;
                    }
                }

                // If alleles is empty, then this is hom ref, skip it
                if (alleles.empty()) {
                    continue;
                }

                alleles.sort();
                if (hasRef) {
                    alleles.push_back(new RawVariant(alleles[0].pos, "", ""));
                }

                if (!alleles.empty()) {
                    gtmap_[sampleIdx][alleles][streamIdx] = &*e;
                }
            }
        }
    }

    void process() {
        for (size_t streamIdx = 0; streamIdx < entries_.size(); ++streamIdx) {
            processEntries(streamIdx);
        }

        for (size_t sampleIdx = 0; sampleIdx < gtmap_.size(); ++sampleIdx) {
            auto& sd = gtmap_[sampleIdx];
            for (auto i = sd.begin(); i != sd.end(); ++i) {
                auto const& rawvs = i->first;
                auto const& who = i->second;
                out_(sampleIdx, sequence_, rawvs, who);
            }
            sd.clear();
        }

        for (size_t streamIdx = 0; streamIdx < entries_.size(); ++streamIdx) {
            entries_[streamIdx].clear();
        }
    }

private:
    typedef boost::ptr_vector<Entry> EntryVector;

    std::vector<std::string> const& sampleNames_;
    std::vector<std::vector<size_t>> sampleIndices_;
    std::vector<FilterType> filterTypes_;
    size_t numStreams_;
    Region region_;
    std::string sequence_;
    std::vector<EntryVector> entries_;
    OutputWriter& out_;
    bool final_;
    std::vector<
        boost::unordered_map<RawVariant::Vector, std::map<size_t, Entry const*>>
        > gtmap_;
};

template<typename HeaderVector, typename OutputWriter>
GenotypeComparator<OutputWriter> makeGenotypeComparator(
            std::vector<std::string> const& sampleNames,
            HeaderVector const& headers,
            std::vector<FilterType> const& filterTypes,
            size_t numStreams,
            OutputWriter& out
            )
{
    return GenotypeComparator<OutputWriter>(sampleNames, headers, filterTypes, numStreams, out);
}

END_NAMESPACE(Vcf)
