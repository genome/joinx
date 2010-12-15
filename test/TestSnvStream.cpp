#include "SnvStream.hpp"
#include "Bed.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(SnvStream, nextSnv) {
    stringstream data(
        "1\t3\t5\tA/T\t43\n"
        "1\t2\t3\tA/T\t44\n"
    );
        
    SnvStream ss("test", data);
    Bed snv;
    ASSERT_TRUE(ss.nextSnv(snv));
    ASSERT_EQ("1", snv.chrom);
    ASSERT_EQ(2u, snv.start);
    ASSERT_EQ(3u, snv.end);
    ASSERT_EQ("A/T", snv.refCall);
    ASSERT_EQ("44", snv.qual);
    ASSERT_TRUE(snv.isSnv());

    ASSERT_EQ(2u, ss.lineNum());
}
