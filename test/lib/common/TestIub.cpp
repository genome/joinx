#include "common/Iub.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <gtest/gtest.h>

using namespace std;

// IUB code definitions taken from http://biocorp.ca/IUB.php
// A    Adenine    A
// C    Cytosine   C
// G    Guanine    G
// T    Thymine    T
// R    AG         puRine
// Y    CT         pYrimidine
// K    GT         Keto
// M    AC         aMino
// S    GC         Strong
// W    AT         Weak
// B    CGT        Not A
// D    AGT        Not C
// H    ACT        Not G
// V    ACG        Not T
// N    AGCT       aNy

TEST(Iub, alleles2bin) {
    ASSERT_EQ(1u, alleles2bin("A"));
    ASSERT_EQ(2u, alleles2bin("C"));
    ASSERT_EQ(4u, alleles2bin("G"));
    ASSERT_EQ(8u, alleles2bin("T"));

    ASSERT_EQ(3u, alleles2bin("AC"));
    ASSERT_EQ(5u, alleles2bin("AG"));
    ASSERT_EQ(9u, alleles2bin("AT"));

    ASSERT_EQ(6u, alleles2bin("CG"));
    ASSERT_EQ(10u, alleles2bin("CT"));
}

TEST(Iub, alleles2iub) {
    ASSERT_EQ('A', alleles2iub("A"));
    ASSERT_EQ('C', alleles2iub("C"));
    ASSERT_EQ('G', alleles2iub("G"));
    ASSERT_EQ('T', alleles2iub("T"));
    ASSERT_EQ('R', alleles2iub("AG"));
    ASSERT_EQ('Y', alleles2iub("CT"));
    ASSERT_EQ('K', alleles2iub("GT"));
    ASSERT_EQ('M', alleles2iub("AC"));
    ASSERT_EQ('M', alleles2iub("AAAAAAACCCCCCCCC"));
    ASSERT_EQ('S', alleles2iub("GC"));
    ASSERT_EQ('W', alleles2iub("AT"));
    ASSERT_EQ('B', alleles2iub("CGT"));
    ASSERT_EQ('D', alleles2iub("AGT"));
    ASSERT_EQ('H', alleles2iub("ACT"));
    ASSERT_EQ('V', alleles2iub("ACG"));
    ASSERT_EQ('N', alleles2iub("AGCT"));

}
