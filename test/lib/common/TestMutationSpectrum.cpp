#include "common/MutationSpectrum.hpp"

#include <gtest/gtest.h>
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

    ASSERT_EQ(9, spectrum.transitions()); 
    ASSERT_EQ(14, spectrum.transversions()); 

    // errors
    ASSERT_THROW(spectrum('X', 'A'), std::runtime_error);
    ASSERT_THROW(spectrum('A', 'N'), std::runtime_error);
    ASSERT_THROW(spectrum('p', 'A'), std::runtime_error);
    ASSERT_THROW(spectrum('A', 'q'), std::runtime_error);
}
