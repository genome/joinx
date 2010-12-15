#pragma once

#include "BedFilterBase.hpp"
#include "Bed.hpp"

// filter entries with no data for the reference sequence allele
class NoReferenceFilter : public BedFilterBase {
public:
    bool _exclude(const Bed& snv) {
        return snv.refCall.size() != 3 ||
            snv.refCall[0] == '\0' || 
            snv.refCall[0] == 'N' || 
            snv.refCall[0] == ' ';
    }
};
