#include "AltNormalizer.hpp"
#include "Entry.hpp"
#include "RawVariant.hpp"
#include "fileformats/Fasta.hpp"

#include <boost/format.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
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
    if (e.chrom() != _seqName) {
        _seqName = e.chrom();
        size_t len = _ref.seqlen(_seqName);
        _sequence = _ref.sequence(_seqName, 1, len);
    }

    RawVariant::Vector rawVariants(RawVariant::processEntry(e));
    int64_t minRefPos = numeric_limits<int64_t>::max();
    int64_t maxRefPos = 0;
    size_t numVariantsProcessed(0);
    for (auto var = rawVariants.begin(); var != rawVariants.end(); ++var) {
        size_t refLen = var->ref.size();
        size_t altLen = var->alt.size();

        // Skip silly alts that are actually the reference
        if (altLen == 0 && refLen == 0)
            continue;

        // Process pure indels only (alts with substitutions are normalized
        // by RawVariant already).
        if ((altLen == 0 || refLen == 0)) {
            if (normalizeRaw(*var, _sequence) != 0u)
                ++numVariantsProcessed;
        }

        minRefPos = min(var->pos, minRefPos);
        maxRefPos = max(var->lastRefPos(), maxRefPos);
    }

    if (!numVariantsProcessed)
        return;

    assert(minRefPos >= 1);

    std::size_t refLen = maxRefPos - minRefPos + 1;

    bool haveEmpty = false;
    for (auto var = rawVariants.begin(); var != rawVariants.end(); ++var) {
        bool eitherEmpty = var->ref.empty() || var->alt.empty();
        if (eitherEmpty && var->pos == minRefPos && refLen == var->ref.size()) {
            haveEmpty = true;
            break;
        }
    }

    if (haveEmpty && minRefPos > 1) {
        --minRefPos;
    }

    // Fetch new reference bases if they changed
    if (minRefPos != int64_t(e.pos()) || maxRefPos != int64_t(e.pos() + e.ref().size())) {
        e._ref = _sequence.substr(minRefPos-1, maxRefPos-minRefPos+1);
    }

    e._pos = minRefPos;

    // Make new alt array and mapping of old alt indices => new alt indices.
    size_t origAltIdx(1);
    size_t newAltIdx(1);
    map<string, size_t> seen;
    map<size_t, size_t> altIndices;
    vector<string> newAlt;
    seen[e.ref()] = 0;

    for (auto var = rawVariants.begin(); var != rawVariants.end(); ++var, ++origAltIdx) {
        assert(uint64_t(var->pos) >= e.pos());

        int64_t headGap = var->pos - e.pos();
        string alt(e.ref().substr(0, headGap));
        alt += var->alt;
        size_t lastRefIdx = var->pos - e.pos() + var->ref.size();
        if (lastRefIdx < e.ref().size())
            alt += e.ref().substr(lastRefIdx);

        auto inserted = seen.insert(make_pair(alt, newAltIdx));
        if (inserted.second) {
            newAlt.push_back(alt);
            if (newAltIdx != origAltIdx)
                altIndices[origAltIdx] = newAltIdx;
            ++newAltIdx;
        } else {
            altIndices[origAltIdx] = inserted.first->second;
        }
    }

    e._alt.swap(newAlt);

    if (!altIndices.empty()) {
        e.sampleData().renumberGT(altIndices);
    }

    if (haveEmpty && minRefPos == 1) {
        // append a base to ref and all alts
        size_t pos = e.pos() + e.ref().size();
        char base = _sequence[pos-1];
        e._ref += base;
        for (auto alt = e._alt.begin(); alt != e._alt.end(); ++alt) {
            *alt += base;
        }
    }
}

END_NAMESPACE(Vcf)
