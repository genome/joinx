#pragma once

#include "RawVariant.hpp"
#include "common/VariantType.hpp"
#include "common/namespaces.hpp"
#include "common/CyclicIterator.hpp"
#include "common/String.hpp"
#include "common/cstdint.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

class Fasta;

BEGIN_NAMESPACE(Vcf)
class Entry;

template<typename RefStringType>
std::size_t normalizeRaw(RawVariant& var, RefStringType const& refseq) {
    // Process only pure indels (not those with substitutions or empty calls).
    if ((!var.ref.empty() && !var.alt.empty())
        || (var.ref.empty() && var.alt.empty()))
        return 0;

    bool deletion = var.alt.empty();
    // Make deletions look like insertions for uniform processing.
    // (Don't you dare forget to swap these back later!)
    if (deletion)
        var.ref.swap(var.alt);

    typedef typename std::string::const_reverse_iterator AltRevIter;
    AltRevIter varBegin = AltRevIter(var.alt.begin());
    AltRevIter varEnd = AltRevIter(var.alt.end());
    auto altCycleBeg = CyclicIterator<AltRevIter>(varEnd, varBegin);
    auto altCycleEnd = CyclicIterator<AltRevIter>(varBegin, varBegin);

    typedef typename RefStringType::const_reverse_iterator RefRevIter;
    RefRevIter revRefBegin(refseq.begin() + var.pos - 1);
    RefRevIter revRefEnd(refseq.begin());

    // Compute the max distance that the alt can be cyclically left shifted
    // while remaining equal to the sequence it overlaps.
    std::size_t shift = commonPrefix(altCycleBeg, altCycleEnd, revRefBegin, revRefEnd);

    // Apply the shift we just calculated.
    var.pos -= shift;
    std::rotate(var.alt.begin(), var.alt.end() - shift % var.alt.size(), var.alt.end());

    // undo alt/ref swap for deletions
    if (deletion)
        var.ref.swap(var.alt);

    return shift;
}

class AltNormalizer {
public:
    typedef Fasta RefSeq;

    AltNormalizer(RefSeq const& ref);

    void normalize(Entry& e);
    void loadReferenceSequence(std::string const& seq);

protected:
    class Impl {
    public:
        Impl(RawVariant::Vector& rawvs, std::string const& refSequence);

        std::size_t normalizeRawVariants();
        bool needPadding() const;

    public: // data
        int64_t minRefPos;
        int64_t maxRefPos;

    private:
        RawVariant::Vector& rawvs_;
        std::string const& refSequence_;
    };

protected:
    RefSeq const& _ref;
    std::string _seqName;
    std::string _sequence;
};

END_NAMESPACE(Vcf)
