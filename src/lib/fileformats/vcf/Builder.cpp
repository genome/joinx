#include "Builder.hpp"
#include "Entry.hpp"
#include "EntryMerger.hpp"

VCF_NAMESPACE_BEGIN

Builder::Builder(const MergeStrategy& mergeStrategy, Header* header, OutputFunc out)
    : _mergeStrategy(mergeStrategy)
    , _header(header)
    , _out(out)
{
}

void Builder::operator()(const Entry& e) {
    e.header();
    if (_entries.empty() || canMerge(e, _entries[0])) {
        _entries.push_back(e);
        return;
    }

    flush();
    _entries.push_back(e);
}

void Builder::flush() {
    EntryMerger merger(_mergeStrategy, _header, &*_entries.begin(), &*_entries.end());
    _out(Entry(merger));
    _entries.clear();
}

bool Builder::canMerge(const Entry& a, const Entry& b) {
    return a.chrom() == b.chrom() && a.pos() == b.pos();
}

VCF_NAMESPACE_END
