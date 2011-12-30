#pragma once

#include "fileformats/TypedStream.hpp"
#include "fileformats/Bed.hpp"

// filter entries with no data for the reference sequence allele
class NoReferenceFilter : public TypedStreamFilterBase<Bed> {
public:
    bool _exclude(const Bed& snv) {
        const std::string& refCall = snv.extraFields()[0];
        return refCall.size() != 3 ||
            refCall[0] == '\0' ||
            refCall[0] == 'N' ||
            refCall[0] == ' ';
    }
};
