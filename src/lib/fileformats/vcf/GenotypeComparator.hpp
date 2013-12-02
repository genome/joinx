#pragma once

#include "Entry.hpp"
#include "Header.hpp"
#include "RawVariant.hpp"
#include "common/Region.hpp"
#include "common/cstdint.hpp"
#include "common/namespaces.hpp"
#include "io/StreamJoin.hpp"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

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
    GenotypeComparator(size_t numStreams, OutputWriter& outputWriter)
        : numStreams_(numStreams)
        , entries_(numStreams)
        , out_(outputWriter)
        , final_(false)
    {
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

    void process() {
        boost::unordered_map<RawVariant::Vector, std::vector<size_t>> gtmap;
        for (size_t i = 0; i < entries_.size(); ++i) {
            for (auto j = entries_[i].begin(); j != entries_[i].end(); ++j) {
                RawVariant::Vector rawvs = RawVariant::processEntry(*j);
                rawvs.sort();
                gtmap[rawvs].push_back(i);
            }
            entries_[i].clear();
        }

        for (auto i = gtmap.begin(); i != gtmap.end(); ++i) {
            auto const& rawv = i->first;
            auto const& who = i->second;
            out_(sequence_, rawv, who);
        }
    }

private:
    typedef boost::ptr_vector<Entry> EntryVector;

    size_t numStreams_;
    Region region_;
    std::string sequence_;
    std::vector<EntryVector> entries_;
    OutputWriter& out_;
    bool final_;
};

template<typename OutputWriter>
GenotypeComparator<OutputWriter> makeGenotypeComparator(size_t numStreams, OutputWriter& out) {
    return GenotypeComparator<OutputWriter>(numStreams, out);
}


END_NAMESPACE(Vcf)
