#pragma once

#include "Sequence.hpp"
#include "common/intconfig.hpp"

#include <cassert>
#include <iostream>
#include <string>

class Bed;

class Variant {
public:
    enum Type {
        SNP,
        DNP,
        INS,
        DEL,
        INVALID
    };

    static std::string typeToString(Type t);

    Variant();
    explicit Variant(const Bed& bed);

    void setVariantSequence(const Sequence& v) {
        _variant = v;
    }

    const std::string& chrom() const {
        return _chrom;
    }

    int64_t start() const {
        return _start;
    }

    int64_t stop() const {
        return _stop;
    }

    int32_t quality() const {
        return _quality;
    }

    int32_t depth() const {
        return _depth;
    }        

    bool valid() const;
    Type type() const;
    std::string typeString() const {
        return typeToString(type());
    }

    void reverseComplement();
    void revCompReference();
    void revCompVariant();

    const Sequence& reference() const;
    const Sequence& variant() const;

    bool isIndel() const {
        return type() == INS || type() == DEL;
    }

    bool positionMatch(const Variant& rhs) const {
        return 
            type() == rhs.type() &&
            start() == rhs.start() &&
            stop() == rhs.stop();
    }

    bool alleleMatch(const Variant& rhs) const {
        return
            reference() == rhs.reference()
            && variant() == rhs.variant();
    }

    std::ostream& toStream(std::ostream& stream) const;

protected:
    Type inferType() const;

protected:
    std::string _chrom;
    int64_t _start;
    int64_t _stop;
    int32_t _quality;
    int32_t _depth;
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

inline const Sequence& Variant::reference() const {
    return _reference;
}

inline const Sequence& Variant::variant() const {
    return _variant;
}

inline bool Variant::valid() const {
    if (_chrom.empty() || _reference.empty() || _variant.empty())
        return false;

    if ((type() == DEL && _variant.data() != "-") || (type() == INS && _reference.data() != "-"))
        return false;

    return true;
}
