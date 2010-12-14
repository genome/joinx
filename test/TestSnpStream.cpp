#include "SnpStream.hpp"
#include "Bed.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(SnpStream, nextSnp) {
    stringstream data(
        "1\t3\t5\tA/T\t43\n"
        "1\t2\t3\tA/T\t44\n"
    );
        
    SnpStream ss("test", data);
    Bed snp;
    ASSERT_TRUE(ss.nextSnp(snp));
    ASSERT_EQ("1", snp.chrom);
    ASSERT_EQ(2u, snp.start);
    ASSERT_EQ(3u, snp.end);
    ASSERT_EQ("A/T", snp.refCall);
    ASSERT_EQ("44", snp.qual);
    ASSERT_TRUE(snp.isSnp());

    ASSERT_EQ(2u, ss.lineNum());
}
