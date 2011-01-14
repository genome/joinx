#include "bedutil/SnvComparator.hpp"
#include "fileformats/BedStream.hpp"
#include "fileformats/Bed.hpp"

#include "MockResultCollector.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

const string BEDA = 
    "1\t2\t3\tA/T\t43\n"
    "1\t3\t4\tA/T\t44\n"
    "19\t3\t4\tA/T\t44\n"
    "19\t3\t4\tA/T\t44\n"
    "X\t2\t3\tA/T\t44\n"
    "Y\t2\t3\tA/T\t44\n";

const string BEDB = 
    "1\t2\t3\tA/T\t43\n"
    "1\t3\t4\tA/T\t44\n"
    "1\t3\t4\tA/T\t44\n"
    "16\t3\t4\tA/T\t44\n"
    "16\t3\t4\tA/T\t44\n"
    "16\t3\t4\tA/T\t44\n"
    "22\t3\t4\tA/T\t44\n"
    "X\t2\t3\tA/T\t44\n"
    "Y\t2\t3\tA/T\t44\n"
    "Y\t2\t3\tA/T\t44\n";

TEST(SnvComparator, intersectAll) {
    stringstream A(BEDA);
    stringstream B(BEDA);
    BedStream sA("a", A);
    BedStream sB("b", B);
    MockResultCollector rc;

    SnvComparator ss(sA, sB, rc);
    ss.exec();

    ASSERT_EQ(6u, rc._hitA.size());
    ASSERT_EQ(6u, rc._hitB.size());
    ASSERT_EQ(0u, rc._missA.size());
    ASSERT_EQ(0u, rc._missB.size());
}

TEST(SnvComparator, intersectSome) {
    stringstream A(BEDA);
    stringstream B(BEDB);
    BedStream sA("a", A);
    BedStream sB("b", B);
    MockResultCollector rc;

    SnvComparator ss(sA, sB, rc);
    ss.exec();

    ASSERT_EQ(4u, rc._hitA.size());
    ASSERT_EQ(6u, rc._hitB.size());
    ASSERT_EQ(2u, rc._missA.size());
    ASSERT_EQ(4u, rc._missB.size());
}
