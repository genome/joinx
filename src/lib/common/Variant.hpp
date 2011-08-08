#pragma once

#include "Sequence.hpp"
#include "Iub.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

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
        _allSequences[1] = v;
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

    float quality() const {
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

    const Sequence& reference() const;
    const Sequence& variant() const;
    const std::vector<Sequence>& allSequences() const;

    bool isIndel() const {
        return type() == INS || type() == DEL;
    }

    bool positionMatch(const Variant& rhs) const {
        return 
            type() == rhs.type() &&
            start() == rhs.start() &&
            stop() == rhs.stop();
    }

    virtual bool alleleMatch(const Variant& rhs) const {
        return
            reference() == rhs.reference()
            && variant() == rhs.variant();
    }

    virtual bool allelePartialMatch(const Variant& rhs) const;
    virtual bool alleleDbSnpMatch(const Variant& rhs) const;

    std::ostream& toStream(std::ostream& stream) const;

protected:
    Type inferType() const;

protected:
    std::string _chrom;
    int64_t _start;
    int64_t _stop;
    float _quality;
    int32_t _depth;
    std::vector<Sequence> _allSequences;
    Type _type;
};

inline void Variant::reverseComplement() {
    for (unsigned i = 0; i < _allSequences.size(); ++i) {
        if (_allSequences[i].data() != "-")
            _allSequences[i] = Sequence(_allSequences[i].reverseComplementData());
    }
}

inline Variant::Type Variant::type() const {
    return _type;
}

inline const Sequence& Variant::reference() const {
    return _allSequences[0];
}

inline const Sequence& Variant::variant() const {
    return _allSequences[1];
}

inline const std::vector<Sequence>& Variant::allSequences() const {
    return _allSequences;
}

inline bool Variant::valid() const {
    if (_chrom.empty() || _allSequences.size() < 2)
        return false;

    if ((type() == DEL && variant().data() != "-") || (type() == INS && reference().data() != "-"))
        return false;

    return true;
}
