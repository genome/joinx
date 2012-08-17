#pragma once
#include <cstdint>
#include <string>
#include "fileformats/vcf/RawVariant.hpp"

class Fasta;

class VariantContig {
public:
    VariantContig(Vcf::RawVariant const& rawvariant, Fasta& reference, int flank, std::string const& seqname);
    
    std::string sequence() const;
    std::string cigar() const;
    int64_t start() const;
    int64_t stop() const;
    
protected:
    std::string _sequence;
    std::string _cigar;
    int64_t _start;
    int64_t _stop;
};
