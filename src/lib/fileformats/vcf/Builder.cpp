#include "Builder.hpp"
#include "ConsensusFilter.hpp"
#include "Entry.hpp"
#include "EntryMerger.hpp"
#include "GenotypeMerger.hpp" // TODO: move DisjointAllelesException out of this header
#include "Header.hpp"
#include "MergeStrategy.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#ifdef DEBUG_VCF_MERGE
# include <iterator>
# include <set>
#endif // DEBUG_VCF_MERGE
#include <utility>

using namespace std;
using namespace std::placeholders;

BEGIN_NAMESPACE(Vcf)

Builder::Builder(
        const MergeStrategy& mergeStrategy,
        Header* header,
        OutputFunc out
        )
    : _mergeStrategy(mergeStrategy)
    , _header(header)
    , _out(out)
{
}

Builder::~Builder() {
    flush();
}

void Builder::operator()(const Entry& e) {
    e.header();
    if (_entries.empty()
        || any_of(_entries.begin(), _entries.end(), bind(&EntryMerger::canMerge, e, _1)))
    {
        _entries.push_back(e);
        return;
    }

    flush();
    _entries.push_back(e);
}

void Builder::operator()(Entry&& e) {
    e.header();
    if (_entries.empty() 
        || any_of(_entries.begin(), _entries.end(), bind(&EntryMerger::canMerge, e, _1)))
    {
        _entries.push_back(std::move(e));
        return;
    }

    flush();
    _entries.push_back(std::move(e));
}

void Builder::writeMergedEntry(Entry& e) const {
    if (_mergeStrategy.clearFilters())
        e.clearFilters();
    _out(e);
}

void Builder::output(Entry* begin, Entry* end) const {
    auto cnsFilt = _mergeStrategy.consensusFilter();
    try {
        EntryMerger merger(_mergeStrategy, _header, begin, end);
        // no merging happened, output each entry individually
        if (!merger.merged()) {
            for (auto e = begin; e != end; ++e) {
                if (cnsFilt)
                    cnsFilt->apply(*e, 0);

                e->reheader(_header);
                writeMergedEntry(*e);
            }
            return;
        }

        // create and output the new merged entry
        //Entry merged(std::move(merger));
        Entry merged(merger);
        if (cnsFilt)
            cnsFilt->apply(merged, &merger.sampleCounts());

        writeMergedEntry(merged);
    } catch (const DisjointGenotypesError& e) {
        // TODO: real logging
        cerr << e.what() << "\nEntries:\n";
        // we'll pick the guy with sourceIndex = 0
        const Entry* chosen = begin;
        for (const Entry* ee = begin; ee != end; ++ee) {
            cerr << *ee << "\n";
            // TODO: NO NO NO NO NO NO take this hack out
            if (ee->header().sourceIndex() == 0)
                chosen = ee;
        }
        if (end - begin == 2) {
            cerr << "Going with entry #" << int(chosen-begin)+1 << ".\n";
            EntryMerger merger(_mergeStrategy, _header, chosen, chosen+1);
            Entry merged(std::move(merger));
            if (cnsFilt)
                cnsFilt->apply(merged, 0);
            writeMergedEntry(merged);
        } else {
            cerr << "More than 2 entries, abort.\n";
            throw;
        }
    }
}

void Builder::flush() {
    if (!_entries.empty()) {
        output(&*_entries.begin(), &*_entries.end());
        _entries.clear();
    }
}

END_NAMESPACE(Vcf)
