#pragma once

#include "Entry.hpp"

#include "common/Sequence.hpp"
#include "common/namespaces.hpp"

#include <vector>
#include <string>

BEGIN_NAMESPACE(Vcf)

// VCF can have a lot of extra padding around a variant
// RawVariant strips this away to expose just the relevant bases
// The sequence name is not represented since it is assumed that
// instances of this class will only be compared on the same
// sequence anyway.
class RawVariant {
public:
    static std::vector<RawVariant> processEntry(Entry const& e) {
        using namespace std;
        vector<RawVariant> rv;
        string const& ref = e.ref();
        for (auto alt = e.alt().begin(); alt != e.alt().end(); ++alt) {
            // find leading bases in the alt that match the reference
            string::size_type varBegin = Sequence::commonPrefix(
                ref.begin(), ref.end(),
                alt->begin(), alt->end()
            );

            string::const_reverse_iterator revRefEnd(ref.begin()+varBegin);
            string::const_reverse_iterator revAltEnd(alt->begin()+varBegin);

            // find trailing bases in the alt that match the reference
            string::size_type csuff = Sequence::commonPrefix(
                ref.rbegin(), revRefEnd,
                alt->rbegin(), revAltEnd
            );
            // what's left in the middle is the actual variant

            string::size_type varEnd = alt->size() - csuff;
            string::size_type refEnd = ref.size() - csuff;
            int64_t pos = e.pos()+varBegin;
            string bases = alt->substr(varBegin, varEnd-varBegin);
            string newRef( ref.substr(varBegin, refEnd-varBegin) );
            rv.emplace_back(pos, newRef, bases);
        }
        return rv;
    }

    RawVariant(int64_t pos, std::string const& ref, std::string const& alt)
        : pos(pos)
        , ref(ref)
        , alt(alt)
    {
    }

    bool operator==(RawVariant const& rhs) const {
        return pos == rhs.pos && ref == rhs.ref && alt == rhs.alt;
    }

    bool operator<(RawVariant const& rhs) const {
        if (pos < rhs.pos) return true;
        if (pos > rhs.pos) return false;

        if (ref < rhs.ref) return true;
        if (ref > rhs.ref) return false;

        if (alt < rhs.alt) return true;
        return false;
    }

public:
    int64_t pos;
    std::string ref;
    std::string alt;
};

END_NAMESPACE(Vcf)
