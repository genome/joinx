#include "common/MutationSpectrum.hpp"

#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>

TEST(TestMutationSpectrum, spectrum) {
    MutationSpectrum spectrum;
    // transitions
    spectrum('A', 'G') = 4;
    spectrum('C', 'T') = 5;

    // transversions
    spectrum('A', 'C') = 2;
    spectrum('A', 'T') = 3;
    spectrum('C', 'A') = 4;
    spectrum('C', 'G') = 5;

    ASSERT_EQ(9u, spectrum.transitions());
    ASSERT_EQ(14u, spectrum.transversions());
    ASSERT_NEAR(9.0/14.0, spectrum.transitionTransversionRatio(), 1e-14);
}

TEST(TestMutationSpectrum, infiniteRatio) {
    MutationSpectrum spectrum;
    double infinity = std::numeric_limits<double>::infinity();
    ASSERT_EQ(infinity, spectrum.transitionTransversionRatio());
    spectrum('A', 'G') = 1;
    ASSERT_EQ(infinity, spectrum.transitionTransversionRatio());
    spectrum('A', 'C') = 1;
    ASSERT_EQ(1.0, spectrum.transitionTransversionRatio());
}

TEST(TestMutationSpectrum, invalidAlleles) {
    MutationSpectrum spectrum;
    for (int i = 0; i < 256; ++i) {
        switch (i) {
            case 'A': case 'C': case 'G': case 'T':
            case 'a': case 'c': case 'g': case 't':
                break;
            default:
                ASSERT_THROW(spectrum(i, 'A'), std::runtime_error)
                    << "char was: " << char(i) << "(" << i << ")";
                ASSERT_THROW(spectrum('A', i), std::runtime_error)
                    << "char was: " << char(i) << "(" << i << ")";
                break;
        }
    }
}
