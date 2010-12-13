#pragma once

#include "SnpFilterBase.hpp"
#include "Bed.hpp"

// filter SNPs with no data for the reference sequence allele
class NoReferenceFilter : public SnpFilterBase {
public:
    bool _exclude(const Bed& snp) {
        return snp.refCall.size() != 3 ||
            snp.refCall[0] == '\0' || 
            snp.refCall[0] == 'N' || 
            snp.refCall[0] == ' ';
    }
};
