#include "common/Integer.hpp"
#include "common/cstdint.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <vector>

TEST(TestInteger, setBits) {
    typedef std::vector<unsigned> Vec;
    EXPECT_EQ(13u, setBits<uint8_t>(Vec{0, 2, 3}));
    EXPECT_EQ(13u, setBits<uint8_t>(Vec{0, 2, 2, 2, 2, 3, 3, 3}));
    EXPECT_EQ(32768u, setBits<uint16_t>(Vec{15}));
    EXPECT_EQ(1048576u, setBits<uint32_t>(Vec{20}));
    EXPECT_EQ(255u, setBits<uint8_t>(Vec{0,1,2,3,4,5,6,7}));
    EXPECT_EQ(0u, setBits<size_t>(Vec{}));
}

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

TEST(TestInteger, unsignedDifference) {
    std::size_t x(200);
    std::size_t y(500);
    uint8_t a(200);
    uint16_t b(500);

    EXPECT_EQ(300u, unsignedDifference(x, y));
    EXPECT_EQ(300u, unsignedDifference(y, x));

    EXPECT_EQ(300u, unsignedDifference(y, a));
    EXPECT_EQ(300u, unsignedDifference(a, y));

    EXPECT_EQ(300u, unsignedDifference(x, b));
    EXPECT_EQ(300u, unsignedDifference(b, x));

    EXPECT_EQ(300u, unsignedDifference(a, b));
    EXPECT_EQ(300u, unsignedDifference(b, a));

    EXPECT_EQ(std::numeric_limits<size_t>::max() - 1,
        unsignedDifference(std::numeric_limits<size_t>::max(), uint8_t(1)));

    EXPECT_EQ(std::numeric_limits<size_t>::max() - 1,
        unsignedDifference(uint8_t(1), std::numeric_limits<size_t>::max()));

    EXPECT_EQ(0u,
        unsignedDifference(
              std::numeric_limits<size_t>::max()
            , std::numeric_limits<size_t>::max()
            )
    );
}
