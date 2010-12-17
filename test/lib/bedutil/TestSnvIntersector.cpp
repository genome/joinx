#include "bedutil/SnvIntersector.hpp"
#include "bedutil/BedStream.hpp"
#include "bedutil/Bed.hpp"

#include "MockResultCollector.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

const string BEDZ = 
    "1\t2\t3\tA/T\t43\n"
    "1\t3\t4\tA/T\t44\n"
    "X\t2\t3\tA/T\t44\n"
    "Y\t2\t3\tA/T\t44\n";

TEST(SnvIntersector, intersect) {
    stringstream A(BEDZ);
    stringstream B(BEDZ);
    BedStream sA("a", A);
    BedStream sB("b", B);
    MockResultCollector rc;

    SnvIntersector ss(sA, sB, rc);
    ss.exec();

    ASSERT_EQ(4u, rc._hit.size());
    ASSERT_EQ(0u, rc._missA.size());
    ASSERT_EQ(0u, rc._missB.size());
}
