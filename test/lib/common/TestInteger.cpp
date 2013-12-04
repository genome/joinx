#include "common/Integer.hpp"
#include "common/cstdint.hpp"

#include <gtest/gtest.h>

#include <limits>

TEST(TestInteger, integerToBinary) {
    EXPECT_EQ("01001000", integerToBinary('H'));
    EXPECT_EQ("01101001", integerToBinary('i'));

    EXPECT_EQ("11", integerToBinary(3, false));
    EXPECT_EQ("00000000000000000000000000000011", integerToBinary<int32_t>(3));
    EXPECT_EQ("00000000000000000000000000000011", integerToBinary<int32_t>(3));
    EXPECT_EQ("01111111111111111111111111111111",
        integerToBinary<int32_t>(std::numeric_limits<int32_t>::max()));
    EXPECT_EQ("11111111111111111111111111111111",
        integerToBinary<int32_t>(std::numeric_limits<uint32_t>::max()));

}
