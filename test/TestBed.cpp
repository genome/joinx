#include "Bed.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(Bed, parse) {
    string snpLine = "1\t2\t3\tA/T\t44";
    Bed snp = Bed::parseLine(snpLine);
    ASSERT_EQ("1", snp.chrom);
    ASSERT_EQ(2, snp.start);
    ASSERT_EQ(3, snp.end);
    ASSERT_EQ("A/T", snp.refCall);
    ASSERT_EQ("44", snp.qual);

    ASSERT_TRUE(snp.isSnp());

    snp.end += 1;
    ASSERT_FALSE(snp.isSnp());
}

TEST(Bed, parseBad) {
    ASSERT_THROW(Bed::parseLine(""), runtime_error);
    ASSERT_THROW(Bed::parseLine("1"), runtime_error);
    ASSERT_THROW(Bed::parseLine("1\t2"), runtime_error);
    ASSERT_THROW(Bed::parseLine("1\t2\t3"), runtime_error);
    ASSERT_THROW(Bed::parseLine("1\t2\t3\tA/T"), runtime_error);
    ASSERT_THROW(Bed::parseLine("1\tK\t3\tA/T\t44"), runtime_error);
    ASSERT_THROW(Bed::parseLine("1\t1\tK\tA/T\t44"), runtime_error);
}

TEST(Bed, cmp) {
    Bed a("1", 1, 1, "A/T", "44");
    Bed b = a;
    b.end++;

    ASSERT_EQ(0, a.cmp(a)) << "bed chromosome sort: 1 == 1";
    ASSERT_GT(0, a.cmp(b)) << "bed chromosome sort: 1 < 2";
    ASSERT_LT(0, b.cmp(a));

    b = a;
    b.start++;
    ASSERT_GT(0, a.cmp(b));
    ASSERT_LT(0, b.cmp(a));

    b = a;
    b.chrom = "2";
    ASSERT_GT(0, a.cmp(b));
    ASSERT_LT(0, b.cmp(a));

    a.chrom = "22";
    b = a;
    b.chrom = "X";
    ASSERT_GT(0, a.cmp(b)) << "bed chromosome sort: 22 < X";
    ASSERT_LT(0, b.cmp(a)) << "bed chromosome sort: X > 22";
}

