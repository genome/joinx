#pragma once

#include "Entry.hpp"

#include "common/Region.hpp"
#include "common/namespaces.hpp"

#include <boost/functional/hash.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>
#include <string>
#include <utility>

class Fasta;

BEGIN_NAMESPACE(Vcf)

// VCF can have a lot of extra padding around a variant
// RawVariant strips this away to expose just the relevant bases
// The sequence name is not represented since it is assumed that
// instances of this class will only be compared on the same
// sequence anyway.
class RawVariant {
public:
    static RawVariant None;
    typedef boost::ptr_vector<RawVariant> Vector;

    static Vector processEntry(Entry const& e) {
        Vector rv;
        for (auto alt = e.alt().begin(); alt != e.alt().end(); ++alt)
            rv.push_back(new RawVariant(e.pos(), e.ref(), *alt));
        return rv;
    }

    RawVariant()
        : pos(0)
    {
    }

    RawVariant(int64_t pos, std::string const& ref, std::string const& alt)
        : pos(pos)
        , ref(ref)
        , alt(alt)
    {
        normalize();
    }

    RawVariant(int64_t pos, std::string&& ref, std::string&& alt)
        : pos(pos)
        , ref(ref)
        , alt(alt)
    {
        normalize();
    }

    Region region() const {
        return Region(pos - 1, pos - 1 + ref.size());
    }

    bool operator==(RawVariant const& rhs) const {
        return pos == rhs.pos && ref == rhs.ref && alt == rhs.alt;
    }

    bool operator!=(RawVariant const& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(RawVariant const& rhs) const {
        if (pos < rhs.pos) return true;
        if (pos > rhs.pos) return false;

        if (ref < rhs.ref) return true;
        if (ref > rhs.ref) return false;

        if (alt < rhs.alt) return true;
        return false;
    }

    int64_t lastRefPos() const {
        return pos + ref.size() - 1;
    }

    int64_t lastAltPos() const {
        return pos + alt.size() - 1;
    }

    std::pair<RawVariant, RawVariant> splitIndelWithSubstitution() const;
    RawVariant mergeIndelWithSubstitution(std::pair<RawVariant, RawVariant> const& vars) const;

    template<typename Container>
    static std::string combineRefAlleles(Container const& x) {
        if (x.empty()) {
            return "";
        }

        int64_t lastRef = 0;
        auto firstRef = x[0].pos;
        for (auto i = x.begin(); i != x.end(); ++i) {
            lastRef = std::max(i->lastRefPos(), lastRef);
        }

        std::string rv(lastRef - firstRef + 1, '.');
        auto lastPos = firstRef;
        for (auto i = x.begin(); i != x.end(); ++i) {
            auto xLast = i->lastRefPos();
            if (xLast < lastPos)
                continue;

            auto thisStart = std::max(lastPos, i->pos);
            auto thisStartIdx = thisStart - firstRef;
            auto thisSkip = thisStart - i->pos;
            auto thisLen = i->ref.size() - thisSkip;
            rv.replace(thisStartIdx, thisLen, i->ref.data() + thisStart - i->pos);
            lastPos = xLast;
        }
        return rv;
    }


protected:
    void normalize();

public:
    int64_t pos;
    std::string ref;
    std::string alt;
};

inline
size_t hash_value(RawVariant const& v) {
    size_t seed = 0;
    boost::hash_combine(seed, v.pos);
    boost::hash_combine(seed, v.ref);
    boost::hash_combine(seed, v.alt);
    return seed;
}

inline
size_t hash_value(RawVariant::Vector const& vs) {
    return boost::hash_range(vs.begin(), vs.end());
}

inline
bool isSimpleIndel(Vcf::RawVariant const& var, size_t maxLength) {
    return var.ref.size() != var.alt.size() &&
        (var.ref.size() == 0 || var.alt.size() == 0) &&
        (var.ref.size() + var.alt.size()) <= maxLength;
}

//Ensure that all bases in the variant match the passed base (i.e. the variant is a homopolymer)
inline
bool allBasesMatch(char a, Vcf::RawVariant const& var) {
    return var.ref.find_first_not_of(a) == std::string::npos &&
        var.alt.find_first_not_of(a) == std::string::npos;
}

inline
std::ostream& operator<<(std::ostream& s,  RawVariant const& rv) {
    s << rv.pos << "\t" << rv.ref << "\t" << rv.alt;
    return s;
}

END_NAMESPACE(Vcf)
