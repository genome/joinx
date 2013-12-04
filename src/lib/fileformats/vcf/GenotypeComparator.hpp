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

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

inline
size_t hash_value(RawVariant::Vector const& vs) {
    return boost::hash_range(vs.begin(), vs.end());
}

template<typename OutputWriter>
class GenotypeComparator {
public:
    template<typename HeaderVector>
    GenotypeComparator(
            std::vector<std::string> const& sampleNames,
            HeaderVector const& headers,
            size_t numStreams,
            OutputWriter& outputWriter
            )
        : sampleNames_(sampleNames)
        , sampleIndices_(numStreams)
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

    void processEntries(size_t streamIdx) {
        auto const& ents = entries_[streamIdx];
        for (auto e = ents.begin(); e != ents.end(); ++e) {
            auto const& sd = e->sampleData();
            RawVariant::Vector rawvs = RawVariant::processEntry(*e);
            if (rawvs.empty()) {
                continue;
            }

            for (size_t sampleIdx = 0; sampleIdx < sampleNames_.size(); ++sampleIdx) {
                RawVariant::Vector alleles;
                GenotypeCall const& call = sd.genotype(sampleIndices_[streamIdx][sampleIdx]);
                for (auto idx = call.indices().begin(); idx != call.indices().end(); ++idx) {
                    std::unique_ptr<RawVariant> rv;
                    if (*idx > 0) {
                        rv.reset(new RawVariant(rawvs[*idx - 1]));
                    }
                    else {
                        rv.reset(new RawVariant);
                        rv->pos = e->pos();
                        rv->alt = e->ref();
                        rv->ref = e->ref();
                    }
                    alleles.push_back(rv.release());
                }
                alleles.sort();
                if (!alleles.empty())
                    gtmap_[sampleIdx][alleles].push_back(streamIdx);
            }
        }
        entries_[streamIdx].clear();
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
    }

private:
    typedef boost::ptr_vector<Entry> EntryVector;

    std::vector<std::string> const& sampleNames_;
    std::vector<std::vector<size_t>> sampleIndices_;
    size_t numStreams_;
    Region region_;
    std::string sequence_;
    std::vector<EntryVector> entries_;
    OutputWriter& out_;
    bool final_;
    std::vector<
        boost::unordered_map<RawVariant::Vector, std::vector<size_t>>
        > gtmap_;
};

template<typename HeaderVector, typename OutputWriter>
GenotypeComparator<OutputWriter> makeGenotypeComparator(
            std::vector<std::string> const& sampleNames,
            HeaderVector const& headers,
            size_t numStreams,
            OutputWriter& out
            )
{
    return GenotypeComparator<OutputWriter>(sampleNames, headers, numStreams, out);
}

END_NAMESPACE(Vcf)
