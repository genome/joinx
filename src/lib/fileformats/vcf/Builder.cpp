#include "Builder.hpp"
#include "Entry.hpp"
#include "EntryMerger.hpp"

#ifdef DEBUG_VCF_MERGE
# include <iostream>
# include <iterator>
# include <set>
#endif // DEBUG_VCF_MERGE

using namespace std;

VCF_NAMESPACE_BEGIN

Builder::Builder(const MergeStrategy& mergeStrategy, Header* header, OutputFunc out)
    : _mergeStrategy(mergeStrategy)
    , _header(header)
    , _out(out)
    , _maxRefLen(0)
{
}

void Builder::push(const Entry& e) {
    _entries.push_back(e);
    _maxRefLen = max(_maxRefLen, _entries.rbegin()->ref().size());
}

void Builder::operator()(const Entry& e) {
    e.header();
    if (_entries.empty() || canMerge(e, _entries[0])) {
        push(e);
        return;
    }

    flush();
    push(e);
}

bool Builder::canMergeWithMaxRef(const Entry& e) const {
    string::size_type maxAltLen(0);
    auto alts = e.alt();
    for (auto alt = alts.begin(); alt != alts.end(); ++alt)
        maxAltLen = max(maxAltLen, alt->size());


    if (e.ref().size() >= _maxRefLen ||
        e.ref().size() > maxAltLen || // entry is all deletions
        maxAltLen > _maxRefLen   // insertions longer than the newreference
        )
    {
        return true;
    }
    return false;
}

vector<Entry>::iterator Builder::partition() {
    auto iter = _entries.begin()-1;
    for (auto i = _entries.begin(); i != _entries.end(); ++i) {
        if (!canMergeWithMaxRef(*i)) {
            swap(*++iter, *i);
        }
    }
    return ++iter;
}

void Builder::output(const Entry* begin, const Entry* end) const {
    EntryMerger merger(_mergeStrategy, _header, begin, end);
    Entry merged(merger);
    _out(merged);

#ifdef DEBUG_VCF_MERGE
    if (merged.alt().size() > 1) {
        cout << "MERGED ALLELES (" << _entries[0].chrom() << ", " << _entries[0].pos() << "): ";
        set<string> origAlleles;
        for (auto i = begin; i != end; ++i) {
            auto alts = i->alt();
            for (auto alt = alts.begin(); alt != alts.end(); ++alt) {
                origAlleles.insert(i->ref() + "," + *alt);
            }
        }
        for (auto i = origAlleles.begin(); i != origAlleles.end(); ++i) {
            if (i != origAlleles.begin())
                cout << " + ";
            cout << *i;
        }
        
        cout << " = " << merged.ref() << ",";
        auto alts = merged.alt();
        for (auto i = alts.begin(); i != alts.end(); ++i) {
            if (i != alts.begin())
                cout << ",";
            cout << *i;
        }
        cout << "\n";
    }
#endif // DEBUG_VCF_MERGE
}


void Builder::flush() {
    if (_entries.size() > 1) {
        output(&*_entries.begin(), &*_entries.end());
    } else if (_entries.size() == 1) {
        _out(_entries[0]);
    }
    _entries.clear();
    _maxRefLen = 0;
}

bool Builder::canMerge(const Entry& a, const Entry& b) {
    return a.chrom() == b.chrom() && a.pos() == b.pos();
}

VCF_NAMESPACE_END
