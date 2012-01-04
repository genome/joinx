#pragma once

#include <cstdint>
#include <array>

// class to keep track of what SNVs have been seen.
// implemented using binary alleles (A=0, C=1, G=2, T=3) and a 4x4.
// this is useful for things such as tracking #transitions/transversions.
class MutationSpectrum {
public:
    MutationSpectrum();

    uint64_t& operator()(uint8_t from, uint8_t to);
    uint64_t const& operator()(uint8_t from, uint8_t to) const;

    uint64_t transitions() const; 
    uint64_t transversions() const; 
    // returns numeric_limits<double>::infinity if transversions=0
    double transitionTransversionRatio() const;

protected:
    int index(uint8_t from, uint8_t to) const;

protected:
    std::array<uint64_t, 16> _mtx;
};
