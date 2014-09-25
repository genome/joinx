#pragma once

#include "fileformats/TypedStream.hpp"
#include "fileformats/Bed.hpp"

// filter entries with no data for the reference sequence allele
class TypeFilter : public TypedStreamFilterBase<Bed> {
public:
    TypeFilter(Bed::Type t) : type_(t) {}

    bool exclude_(const Bed& snv) {
        return type_ != snv.type();
    }

protected:
    Bed::Type type_;
};
