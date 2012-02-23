#include "AltNormalizer.hpp"
#include "Entry.hpp"
#include "common/Sequence.hpp"
#include "fileformats/Fasta.hpp"

#include <iostream>

#include <boost/format.hpp>
#include <algorithm>
#include <iterator>
#include <stdexcept>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)

namespace {
    template<typename RevIterA, typename RevIterB>
    string::size_type leftShift(
        RevIterA refBeg,
        RevIterA refEnd,
        RevIterB varBeg,
        RevIterB varEnd)
    {
        string::size_type shift(0);
        string::size_type len = distance(varBeg, varEnd);
        while (refBeg != refEnd) {
            string::size_type s = Sequence::commonPrefix(
                refBeg, refEnd,
                varBeg, varEnd);
            shift += s;
            refBeg += s;
            if (s < len)
                break;
        }
        return shift;
    }


}

AltNormalizer::AltNormalizer(RefSeq const& ref)
    : _ref(ref)
{
}

void AltNormalizer::normalize(Entry& e) {
    if (e.pos() == 1)
        return;

    vector<RefEdit> shifts(e.alt().size());
    string::size_type refLen = e.ref().size();
    size_t idx(0);
    uint64_t minRefPos = numeric_limits<uint64_t>::max();
    uint64_t maxRefPos = 0;
    for (auto alt = e.alt().begin(); alt != e.alt().end(); ++alt, ++idx) {
        // for indels, we want 1 base of padding before the indel in the output
        string::size_type refPadding(1);
        // what do you look like, variant?
        if (alt->size() < refLen) { // i look like a deletion
            shifts[idx] = moveDeletion(e, idx);
        } else if (alt->size() > refLen) { // i look like an insertion
            shifts[idx] = moveInsertion(e, idx);
        } else { // i look like a substitution
            refPadding = 0; // don't pad substitutions
            string::size_type prefix = Sequence::commonPrefix(e.ref(), *alt);
            shifts[idx] = RefEdit(e.pos()+prefix, alt->size()-prefix, e.pos()+prefix, alt->substr(prefix));
        }

        uint64_t refPos = std::min(shifts[idx].lastRef, shifts[idx].pos - refPadding);
        uint64_t refLen = shifts[idx].lastRef - refPos + 1;
        minRefPos = min(refPos, minRefPos);
        maxRefPos = max(refPos+refLen, maxRefPos);

        string ref = _ref.sequence(e.chrom(), refPos, refLen);
        string alt = ref;
        if (refPos+refLen-1 >= shifts[idx].pos) {
            string::size_type splicePos = refLen - (shifts[idx].lastRef - shifts[idx].pos + 1);
            alt.replace(splicePos, shifts[idx].len, shifts[idx].bases);
        } else
            alt += shifts[idx].bases;
    }
    e._pos = minRefPos;
    e._ref = _ref.sequence(e.chrom(), minRefPos, maxRefPos-minRefPos);
    
    idx = 0;
    for (auto alt = e.alt().begin(); alt != e.alt().end(); ++alt, ++idx) {
        e._alt[idx] = e._ref;
        if (maxRefPos >= shifts[idx].pos) {
            string::size_type splicePos = shifts[idx].pos - minRefPos;
            e._alt[idx].replace(splicePos, shifts[idx].len, shifts[idx].bases);
        } else
            e._alt[idx] += shifts[idx].bases;
    }
}

AltNormalizer::RefEdit AltNormalizer::moveInsertion(Entry& e, size_t idx) const {
    auto const& ref = e.ref();
    auto const& alt = e.alt()[idx]; // alternate allele

    // extract the inserted bases
    string ins = alt.substr(ref.size());

    // Find last base of the "padding", aka the base before the insertion begins
    // e.g., for REF: TC, ALT: TCGG this points to the C in ALT and is
    // a REVERSE iterator (i.e., *(padLast+1) = T)
    string::const_iterator padLastFwd(alt.begin() + ref.size());
    string::const_reverse_iterator padLastRev(padLastFwd);

    // 1-based position in the sequence where the inserted bases begin
    uint64_t insPos = e.pos() + ref.size();

    // check for substitutions in the pad
    string::size_type padEqual = Sequence::commonPrefix(
        padLastRev, alt.rend(),
        ref.rbegin(), ref.rend()
        );

    if (padEqual == ref.size()) {
        // there are no substitutions in the pad
        string::size_type shift = leftShift(
            ref.rbegin(), ref.rend(),
            ins.rbegin(), ins.rend());

        insPos -= shift;
        if (shift)
            std::rotate(ins.begin(), ins.end(), ins.end()-shift%ins.size());

        // we didn't shift all the way to the beginning of ref
        if (shift != ref.size())
            return RefEdit(insPos, ins.size(), insPos-1, ins);

        string::size_type refLen = ref.size() * 2;
        while (insPos-refLen > 1) {
            if (refLen >= insPos)
                refLen = insPos-1;
            string eref = _ref.sequence(e.chrom(), insPos-refLen, refLen);
            string::size_type thisShift = leftShift(
                eref.rbegin(), eref.rend(),
                ins.rbegin(), ins.rend()
                );
            shift += thisShift;
            insPos -= thisShift;
            if (thisShift)
                std::rotate(ins.begin(), ins.end(), ins.end()-thisShift%ins.size());
            if (thisShift != refLen)
                break;
            refLen *= 2;
        }

        return RefEdit(insPos, ins.size(), insPos-1, ins);
    } else if (padEqual > 0) {
        // there are substitutions in the pad, we cannot shift past them

        // see if there is any leading reference we can drop
        string::size_type firstVariantBase = Sequence::commonPrefix(
            ref.begin(), ref.end(),
            alt.begin(), padLastFwd
            );

        string::size_type shift = leftShift(padLastRev, padLastRev+padEqual, ins.rbegin(), ins.rend());
        if (shift)
            std::rotate(ins.begin(), ins.end(), ins.end()-shift%ins.size());
        string::size_type padLen = distance(alt.begin(), padLastFwd);
        string bases = alt.substr(firstVariantBase, padLen-shift-firstVariantBase);
        bases += ins;
        return RefEdit(
            e.pos()+firstVariantBase,
            alt.size() - shift,
            e.pos()+firstVariantBase,
            bases
            );
    }
    return RefEdit(e.pos(), alt.size(), e.pos()+ref.size(), alt);
}

AltNormalizer::RefEdit AltNormalizer::moveDeletion(Entry& e, size_t idx) const {
    auto const& ref = e.ref();
    auto const& alt = e.alt()[idx]; // alternate allele

    // extract the deleted bases
    string del = ref.substr(alt.size());

    string::const_iterator padLastFwd(alt.end());
    string::const_reverse_iterator padLastRev(alt.rbegin());

    // 1-based position in the sequence where the deleted bases begin
    uint64_t delPos = e.pos() + alt.size();

    // check for substitutions in the pad
    string::size_type padEqual = Sequence::commonPrefix(
        padLastRev, alt.rend(),
        ref.rbegin()+del.size(), ref.rend()
        );

    if (padEqual+del.size() == ref.size()) {
        // there are no substitutions in the pad
        string::size_type shift = leftShift(
            alt.rbegin(), alt.rend(),
            del.rbegin(), del.rend());

        delPos -= shift;
        if (shift)
            std::rotate(del.begin(), del.end(), del.end()-shift%del.size());

        // we didn't shift all the way to the beginning of alt
        if (shift != alt.size())
            return RefEdit(delPos, del.size(), delPos+del.size()-1, "");

        string::size_type refLen = ref.size() * 2;
        while (delPos-refLen > 1) {
            if (refLen >= delPos)
                refLen = delPos-1;
            string eref = _ref.sequence(e.chrom(), delPos-refLen, refLen);
            string::size_type thisShift = leftShift(
                eref.rbegin(), eref.rend(),
                del.rbegin(), del.rend()
                );
            shift += thisShift;
            delPos -= thisShift;
            if (thisShift)
                std::rotate(del.begin(), del.end(), del.end()-thisShift%del.size());
            if (thisShift != refLen)
                break;
            refLen *= 2;
        }

        return RefEdit(delPos, del.size(), delPos+del.size()-1, "");
    } else if (padEqual > 0) {
        // there are substitutions in the pad, we cannot shift past them

        // see if there is any leading reference we can drop
        string::size_type firstVariantBase = Sequence::commonPrefix(
            ref.begin(), ref.end(),
            alt.begin(), padLastFwd
            );

        string::size_type shift = leftShift(padLastRev, padLastRev+padEqual, del.rbegin(), del.rend());
        if (shift)
            std::rotate(del.begin(), del.end(), del.end()-shift%del.size());
        string::size_type padLen = distance(alt.begin(), padLastFwd);
        string bases = alt.substr(firstVariantBase, padLen-shift-firstVariantBase);
        return RefEdit(
            e.pos()+firstVariantBase,
            bases.size() + del.size(),
            e.pos()+firstVariantBase+del.size(),
            bases
            );
    }
    return RefEdit(e.pos(), alt.size(), e.pos()+ref.size(), alt);
}

END_NAMESPACE(Vcf)
