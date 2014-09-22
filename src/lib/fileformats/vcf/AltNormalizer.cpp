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
#include <utility>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)


AltNormalizer::Impl::Impl(Entry& entry, std::string const& refSequence)
    : entry_(entry)
    , rawvs_(RawVariant::processEntry(entry))
    , refSequence_(refSequence)
    , minRefPos(numeric_limits<int64_t>::max())
    , maxRefPos(0)
{
}

void AltNormalizer::Impl::normalize() {
    // if any variants were actually normalized,
    if (normalizeRawVariants() != 0)
        editEntry();
}

void AltNormalizer::Impl::editEntry() {
    int64_t origPos = entry_.pos();
    int64_t origEnd = origPos + entry_.ref().size();
    bool refChanged = minRefPos != origPos || maxRefPos != origEnd;

    // Fetch new reference bases if they changed
    if (refChanged)
        entry_._ref = refSequence_.substr(minRefPos-1, maxRefPos-minRefPos+1);

    entry_._pos = minRefPos;

    // add the required padding to make each raw variant's ref allele match
    // that of the final entry.
    vector<string> newAlt(entry_.alt().size());
    for (std::size_t i = 0; i < rawvs_.size(); ++i) {
        auto const& var = rawvs_[i];
        assert(uint64_t(var.pos) >= entry_.pos());

        int64_t headGap = var.pos - minRefPos;
        string alt(entry_.ref().substr(0, headGap) + var.alt);

        size_t lastRefIdx = var.pos - minRefPos + var.ref.size();
        if (lastRefIdx < entry_.ref().size())
            alt += entry_.ref().substr(lastRefIdx);

        newAlt[i] = std::move(alt);
    }

    entry_._alt.swap(newAlt);
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

    if (numVariantsMoved > 0 && needPadding())
        addPadding();

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

void AltNormalizer::Impl::addPadding() {
    if (minRefPos > 1)
        --minRefPos;
    else
        ++maxRefPos;
}

AltNormalizer::AltNormalizer(RefSeq const& ref)
    : ref_(ref)
{
}

void AltNormalizer::loadReferenceSequence(std::string const& seq) {
    if (seq != seqName_) {
        seqName_ = seq;
        size_t len = ref_.seqlen(seqName_);
        sequence_ = ref_.sequence(seqName_, 1, len);
    }
}

void AltNormalizer::normalize(Entry& e) {
    loadReferenceSequence(e.chrom());
    Impl impl(e, sequence_);
    impl.normalize();
}

END_NAMESPACE(Vcf)
