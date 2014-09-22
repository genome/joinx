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


AltNormalizer::Impl::Impl(RawVariant::Vector& rawvs, std::string const& refSequence)
    : minRefPos(numeric_limits<int64_t>::max())
    , maxRefPos(0)
    , rawvs_(rawvs)
    , refSequence_(refSequence)
{
}

std::size_t AltNormalizer::Impl::normalizeRawVariants() {
    size_t numVariantsMoved = 0;
    for (auto var = rawvs_.begin(); var != rawvs_.end(); ++var) {
        size_t refLen = var->ref.size();
        size_t altLen = var->alt.size();

        // Skip silly alts that are actually the reference
        if (altLen == 0 && refLen == 0)
            continue;

        // Only pure indels will move.
        if (normalizeRaw(*var, refSequence_) != 0u)
            ++numVariantsMoved;

        minRefPos = min(var->pos, minRefPos);
        maxRefPos = max(var->lastRefPos(), maxRefPos);
    }

    assert(minRefPos >= 1);

    return numVariantsMoved;
}

bool AltNormalizer::Impl::needPadding() const {
    // We need to add padding if there is a variant where one of the alleles
    // (either ref or alt) spans the entirety of the reference allele and the
    // other is empty.
    std::size_t refLen = maxRefPos - minRefPos + 1;
    for (auto var = rawvs_.begin(); var != rawvs_.end(); ++var) {
        bool eitherEmpty = var->ref.empty() || var->alt.empty();
        if (eitherEmpty && var->pos == minRefPos && refLen == var->ref.size())
            return true;
    }
    return false;
}


AltNormalizer::AltNormalizer(RefSeq const& ref)
    : _ref(ref)
{
}

void AltNormalizer::loadReferenceSequence(std::string const& seq) {
    if (seq != _seqName) {
        _seqName = seq;
        size_t len = _ref.seqlen(_seqName);
        _sequence = _ref.sequence(_seqName, 1, len);
    }
}

void AltNormalizer::normalize(Entry& e) {
    loadReferenceSequence(e.chrom());
    RawVariant::Vector rawVariants(RawVariant::processEntry(e));
    Impl impl(rawVariants, _sequence);
    size_t numVariantsMoved = impl.normalizeRawVariants();
    if (numVariantsMoved == 0)
        return;

    auto minRefPos = impl.minRefPos;
    auto maxRefPos = impl.maxRefPos;
    bool needPadding = impl.needPadding();

    if (needPadding && minRefPos > 1) {
        --minRefPos;
        needPadding = false;
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

    if (needPadding && minRefPos == 1) {
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
