#pragma once

#include "SnvFilterBase.hpp"
#include "Bed.hpp"

// filter SNVs with no data for the reference sequence allele
class NoReferenceFilter : public SnvFilterBase {
public:
    bool _exclude(const Bed& snv) {
        return snv.refCall.size() != 3 ||
            snv.refCall[0] == '\0' || 
            snv.refCall[0] == 'N' || 
            snv.refCall[0] == ' ';
    }
};
