#pragma once

#include "fileformats/TypedStream.hpp"
#include "fileformats/Bed.hpp"

// filter entries with no data for the reference sequence allele
class TypeFilter : public TypedStreamFilterBase<Bed> {
public:
    TypeFilter(Bed::Type t) : _type(t) {}

    bool _exclude(const Bed& snv) {
        return _type != snv.type();
    }

protected:
    Bed::Type _type;
};
