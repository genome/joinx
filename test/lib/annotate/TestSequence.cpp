#include "annotate/Sequence.hpp"

#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(TranscriptStructure, parseLine) {
    string input = "acgtrymkswhbvdnx";
    string expected = "XNHBVDWSMKRYACGT";

    Sequence seq(input);
    ASSERT_EQ(expected, seq.reverseComplementData());
}
