#pragma once

#include "fileformats/vcf/ConsensusFilter.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/EntryMerger.hpp"
#include "fileformats/vcf/GenotypeMerger.hpp" // TODO: move DisjointAllelesException out of this header
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/MergeStrategy.hpp"

#include <memory>
#include <vector>

template<typename OutputFunc>
class VcfEntryMerger {
public:
    typedef std::unique_ptr<Vcf::Entry> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    VcfEntryMerger(
              OutputFunc& out
            , Vcf::Header* mergedHeader
            , Vcf::MergeStrategy const& mergeStrategy
            )
        : out_(out)
        , mergedHeader_(mergedHeader)
        , mergeStrategy_(mergeStrategy)
    {}

    void writeMergedEntry(Vcf::Entry& e) {
        if (mergeStrategy_.clearFilters())
            e.clearFilters();
        out_(e);
    }

    void operator()(ValuePtrVector entries) {
        using namespace Vcf;
        // FIXME: rewrite EntryMerger to work with pointers so we don't
        // have to move the entries into this vector
        std::vector<Vcf::Entry> rawEntries(entries.size());
        for (std::size_t i = 0; i < entries.size(); ++i) {
            rawEntries[i] = std::move(*entries[i]);
        }
        entries.clear();

        auto begin = rawEntries.data();
        auto end = rawEntries.data() + rawEntries.size();

        EntryMerger merger(mergeStrategy_ , mergedHeader_ , begin , end);

        auto cnsFilt = mergeStrategy_.consensusFilter();
        try {
            EntryMerger merger(mergeStrategy_, mergedHeader_, begin, end);
            // no merging happened, output each entry individually
            if (!merger.merged()) {
                for (auto e = rawEntries.begin(); e != rawEntries.end(); ++e) {
                    if (cnsFilt)
                        cnsFilt->apply(*e, 0);

                    e->reheader(mergedHeader_);
                    writeMergedEntry(*e);
                }
                return;
            }

            // create and output the new merged entry
            Entry merged(std::move(merger));
            if (cnsFilt)
                cnsFilt->apply(merged, &merger.sampleCounts());

            writeMergedEntry(merged);
        } catch (const DisjointGenotypesError& e) {
            // FIXME: do something
            // probably try to figure out which entries are fighting,
            // take one out, and reject it.
            throw;
        }
    }


private:
    OutputFunc& out_;
    Vcf::Header* mergedHeader_;
    Vcf::MergeStrategy const& mergeStrategy_;
};

template<typename OutputFunc>
VcfEntryMerger<OutputFunc>
makeVcfEntryMerger(
          OutputFunc& out
        , Vcf::Header* mergedHeader
        , Vcf::MergeStrategy const& mergeStrategy
        )
{
    return VcfEntryMerger<OutputFunc>(out, mergedHeader, mergeStrategy);
}

