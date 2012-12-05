#include "RawVariant.hpp"
#include "fileformats/Fasta.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

using namespace std;

BEGIN_NAMESPACE(Vcf)

RawVariant RawVariant::None(0, "", "");

void RawVariant::normalize() {
    // find leading bases in the alt that match the reference
    string::size_type varBegin = Sequence::commonPrefix(
        ref.begin(), ref.end(),
        alt.begin(), alt.end()
    );

    string::const_reverse_iterator revRefBeg(ref.rbegin());
    string::const_reverse_iterator revRefEnd(ref.begin()+varBegin);
    string::const_reverse_iterator revAltBeg(alt.rbegin());
    string::const_reverse_iterator revAltEnd(alt.begin()+varBegin);

    // find trailing bases in the alt that match the reference
    string::size_type csuff = Sequence::commonPrefix(
        revRefBeg, revRefEnd,
        revAltBeg, revAltEnd
    );
    // what's left in the middle is the actual variant

    string::size_type varEnd = alt.size() - csuff;
    string::size_type refEnd = ref.size() - csuff;
    this->pos += varBegin;
    this->alt = alt.substr(varBegin, varEnd-varBegin);
    this->ref = ref.substr(varBegin, refEnd-varBegin);
}

std::pair<RawVariant, RawVariant> RawVariant::splitIndelWithSubstitution() const {
    if (ref.size() == 0 || alt.size() == 0 || ref.size() == alt.size()) {
        return make_pair(*this, None);
    }

    string const* refStr(0);
    string const* altStr(0);

    if (alt.size() > ref.size()) {
        refStr = &ref;
        altStr = &alt;
    } else {
        refStr = &alt;
        altStr = &ref;
    }

    // insertion with substitution
    typedef string::const_reverse_iterator RevIter;
    string::const_iterator substEnd = altStr->begin() + refStr->size();
    size_t substCommonSuffix = Sequence::commonPrefix(
        RevIter(substEnd), altStr->rend(),
        refStr->rbegin(), refStr->rend()
        );

    size_t substLen = refStr->size() - substCommonSuffix;
    RawVariant substVar(pos, refStr->substr(0, substLen), altStr->substr(0, substLen));

    size_t indelCommonPrefix = Sequence::commonPrefix(
        altStr->begin() + substLen, altStr->end(),
        refStr->begin() + substLen, refStr->end()
    );
    size_t indelPos = substLen + indelCommonPrefix;
    RawVariant indelVar(pos+indelPos, refStr->substr(indelPos), altStr->substr(indelPos));
    if (alt.size() < ref.size()) {
        substVar.ref.swap(substVar.alt);
        indelVar.ref.swap(indelVar.alt);
    }

    return make_pair(substVar, indelVar);
}

RawVariant RawVariant::mergeIndelWithSubstitution(std::pair<RawVariant, RawVariant> const& vars) const {
    RawVariant const& a = vars.first;
    RawVariant const& b = vars.second;

    if (a == None) return b;
    if (b == None) return a;

    if (a.pos > b.pos) {
        throw runtime_error("Attempted to merge raw variants, but a.pos > b.pos");
    }

    string newRef = a.ref;
    string newAlt = a.alt;
    int64_t gap = b.pos - a.lastRefPos() - 1;
    if (gap > 0) {
        string refGap(ref.substr(a.ref.size(), gap));
        newRef.append(refGap);
        newAlt.append(refGap);
    } else if (gap < 0) {
        throw runtime_error("Attempted to merge overlapping raw variants!");
    }

    newRef.append(b.ref);
    newAlt.append(b.alt);
    
    return RawVariant(pos, std::move(newRef), std::move(newAlt));
}

END_NAMESPACE(Vcf)
