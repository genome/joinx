#include "AltNormalizer.hpp"
#include "Entry.hpp"
#include "RawVariant.hpp"
#include "common/CyclicIterator.hpp"
#include "common/Sequence.hpp"
#include "fileformats/Fasta.hpp"

#include <boost/format.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)

AltNormalizer::AltNormalizer(RefSeq const& ref)
    : _ref(ref)
{
}

void AltNormalizer::normalize(Entry& e) {
    Entry origEntry(e);

    if (e.chrom() != _seqName) {
        _seqName = e.chrom();
        size_t len = _ref.seqlen(_seqName);
        _sequence = _ref.sequence(_seqName, 1, len);
    }

    vector<RawVariant> rawVariants(RawVariant::processEntry(e));
    size_t minRefPos = numeric_limits<size_t>::max();
    size_t maxRefPos = 0;
    bool haveIndel = false;
    for (auto var = rawVariants.begin(); var != rawVariants.end(); ++var) {
        size_t refLen = var->ref.size();
        size_t altLen = var->alt.size();

        string::const_iterator varBegin;
        string::const_iterator varEnd;

        // Process pure indels only (alts with substitutions are normalized
        // by RawVariant already).
        if ((altLen == 0 || refLen == 0) && (altLen != refLen)) {
            haveIndel = true;

            // Make deletions look like insertions for uniform processing.
            // Don't forget to swap it back later!
            if (altLen == 0)
                var->ref.swap(var->alt);

            varBegin = var->alt.begin();
            varEnd = var->alt.end();
            size_t shift = leftShift(varBegin, varEnd, var->pos);
            var->pos -= shift;
            std::rotate(var->alt.begin(), var->alt.end() - shift % var->alt.size(), var->alt.end());

            // swap back for deletions
            if (altLen == 0)
                var->ref.swap(var->alt);
        }
        
        minRefPos = min(size_t(var->pos), minRefPos);
        maxRefPos = max(size_t(var->lastRefPos()), maxRefPos);
    }

    assert(minRefPos >= 1);
    if (haveIndel && minRefPos > 1) {
        --minRefPos;
    }

    // Fetch new reference bases if they changed
    if (minRefPos != e.pos() || maxRefPos != e.pos() + e.ref().size()) {
        e._ref = _sequence.substr(minRefPos-1, maxRefPos-minRefPos+1);
    }

    e._pos = minRefPos;

    size_t idx(0);
    map<string, size_t> seen;
    map<size_t, size_t> altIndices;
    vector<string> newAlt;
    for (auto var = rawVariants.begin(); var != rawVariants.end(); ++var, ++idx) {
        size_t altNumber = idx + 1;
        assert(uint64_t(var->pos) >= e.pos());

        int64_t headGap = var->pos - e.pos();
        string alt(e.ref().substr(0, headGap));
        alt += var->alt;
        size_t lastRefIdx = var->pos - e.pos() + var->ref.size();
        if (lastRefIdx < e.ref().size())
            alt += e.ref().substr(lastRefIdx);
        if (alt == e.ref()) {
            altIndices[altNumber] = 0;
        } else {
            auto inserted = seen.insert(make_pair(alt, altNumber));
            if (inserted.second)
                newAlt.push_back(alt);
            else
                altIndices[altNumber] = inserted.first->second;
        }
    }
    e._alt.swap(newAlt);

    if (!altIndices.empty()) {
        cerr << "Renumbering:\n\t" << origEntry << "\n";
        for (auto i = altIndices.begin(); i != altIndices.end(); ++i)
            cerr << "\t" << i->first << " => " << i->second << "\n";
        e._sampleData.renumberGT(altIndices);
        cerr << "\t" << e << "\n";
    }

    if (haveIndel && minRefPos == 1) {
        // append a base to ref and all alts
        size_t pos = e.pos() + e.ref().size();
        char base = _sequence[pos-1];
        e._ref += base;
        for (auto alt = e._alt.begin(); alt != e._alt.end(); ++alt) {
            *alt += base;
        }
    }
}

size_t AltNormalizer::leftShift(
    std::string::const_iterator varBegin,
    std::string::const_iterator varEnd,
    size_t varPos
    )
{
    typedef string::const_reverse_iterator RevIter;
    auto altCycleBeg = CyclicIterator<RevIter>(RevIter(varEnd), RevIter(varBegin));
    auto altCycleEnd = CyclicIterator<RevIter>(RevIter(varBegin), RevIter(varBegin));
    RevIter revRefBegin(_sequence.begin() + varPos - 1);
    RevIter revRefEnd(_sequence.begin());
    return Sequence::commonPrefix(altCycleBeg, altCycleEnd, revRefBegin, revRefEnd);
}

END_NAMESPACE(Vcf)
