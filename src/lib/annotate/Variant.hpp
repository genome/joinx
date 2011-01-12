#pragma once

#include "Sequence.hpp"
#include "bedutil/Bed.hpp"

#include <cassert>
#include <string>

class Variant {
public:
    enum Type {
        SNP,
        DNP,
        INS,
        DEL,
        INVALID
    };

    Variant();
    Variant(const Bed* bed);

    int64_t start() const {
        assert(_bed != NULL);
        return _bed->start;
    }

    int64_t end() const {
        assert(_bed != NULL);
        return _bed->end;
    }

    bool valid() const;
    Type type() const;

    void reverseComplement();
    void revCompReference();
    void revCompVariant();

    const Sequence& reference() const;
    const Sequence& variant() const;

protected:
    Type inferType() const;


protected:
    const Bed* _bed; // NOTE! this is passed in and expected to live as long as this object
    Sequence _reference;
    Sequence _variant;
    Type _type;
};

inline void Variant::reverseComplement() {
    if (type() != DEL) 
        _variant = Sequence(_variant.reverseComplementData());
    if (type() != Variant::INS)
        _reference = Sequence(_reference.reverseComplementData());
}

inline void Variant::revCompVariant() {
}

inline Variant::Type Variant::type() const {
    return _type;
}

inline bool Variant::valid() const {
    if (!_bed || _bed->chrom.empty() || _reference.empty() || _variant.empty())
        return false;

    if ((type() == DEL && _variant.data() != "-") || (type() == INS && _reference.data() != "-"))
        return false;

    return true;
}
