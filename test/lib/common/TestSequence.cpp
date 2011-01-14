#include "common/Sequence.hpp"

#include <sstream>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(Sequence, reverseComplement) {
    string input = "acgtrymkswhbvdnx";
    string expected = "XNHBVDWSMKRYACGT";

    Sequence seq(input);
    ASSERT_EQ(expected, seq.reverseComplementData());
}

TEST(Sequence, fromStream) {
    stringstream ss("ACGTACGTACGT");
    string expected = "ACGTA";
    Sequence seq(ss, 5); 
    ASSERT_EQ(expected, seq.data()); 
}
