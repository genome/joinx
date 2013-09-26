#pragma once

#include "common/CigarString.hpp"
#include "common/cstdint.hpp"
#include "fileformats/vcf/RawVariant.hpp"

#include <string>

class Fasta;

class VariantContig {
public:
    VariantContig(
        Vcf::RawVariant const& var,
        Fasta& ref,
        int flank,
        std::string const& seqname);

    std::string sequence() const;
    std::string cigar() const;
    int64_t start() const;
    int64_t stop() const;

protected:
    std::string _sequence;
    CigarString _cigar;
    int64_t _start;
    int64_t _stop;
};

inline std::string VariantContig::sequence() const {
    return _sequence;
}

inline std::string VariantContig::cigar() const {
    return _cigar;
}

inline int64_t VariantContig::start() const {
    return _start;
}

inline int64_t VariantContig::stop() const {
    return _stop;
}
