#include "processors/SnvComparator.hpp"
#include "fileformats/Bed.hpp"
#include "io/InputStream.hpp"
#include "fileformats/TypedStream.hpp"

#include "MockResultCollector.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

const string BEDA =
    "1\t2\t3\tA/T\t43\n"
    "1\t3\t4\tA/T\t44\n"
    "19\t3\t4\tA/T\t44\n"
    "19\t3\t4\tA/T\t44\n"
    "X\t2\t3\tA/T\t44\n"
    "Y\t1\t2\tA/T\t44\n";

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

namespace {
    typedef boost::function<void(const BedHeader*, string&, Bed&)> Extractor;
    Extractor exA = boost::bind(&Bed::parseLine, _1, _2, _3, 2);
    Extractor exB = boost::bind(&Bed::parseLine, _1, _2, _3, 0);
    typedef TypedStream<Bed, boost::function<void(const BedHeader*, std::string&, Bed&)> > BedReader;
}

TEST(SnvComparator, intersectAll) {
    stringstream A(BEDA);
    stringstream B(BEDA);
    InputStream streamA("A", A);
    InputStream streamB("B", B);
    BedReader sA(exA, streamA);
    BedReader sB(exB, streamB);
    MockResultCollector rc;

    SnvComparator<BedReader, MockResultCollector> ss(sA, sB, rc);
    ss.exec();

    ASSERT_EQ(6u, rc._hitA.size());
    ASSERT_EQ(6u, rc._hitB.size());
    ASSERT_EQ(0u, rc._missA.size());
    ASSERT_EQ(0u, rc._missB.size());
}

TEST(SnvComparator, intersectSome) {
    stringstream A(BEDA);
    stringstream B(BEDB);
    InputStream streamA("A", A);
    InputStream streamB("B", B);
    BedReader sA(exA, streamA);
    BedReader sB(exB, streamB);

    MockResultCollector rc;

    SnvComparator<BedReader, MockResultCollector> ss(sA, sB, rc);
    ss.exec();

    ASSERT_EQ(3u, rc._hitA.size());
    ASSERT_EQ(4u, rc._hitB.size());
    ASSERT_EQ(3u, rc._missA.size());
    ASSERT_EQ(6u, rc._missB.size());
}
