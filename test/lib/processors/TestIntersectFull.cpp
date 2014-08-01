#include "processors/IntersectFull.hpp"
#include "fileformats/Bed.hpp"
#include "io/InputStream.hpp"
#include "fileformats/TypedStream.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

using namespace std;

namespace {
    const string BEDA =
        "1\t2\t2\t*/CC\t30\t30\n"
        "1\t2\t2\t*/CCC\t30\t30\n"
        "1\t5\t8\tTTT/*\t30\t30\n"
        "1\t5\t8\tCCC/*\t30\t30";

    const string BEDB =
        "1\t2\t2\t*/CC\t30\t30\n"
        "1\t2\t2\t*/CCC\t30\t30\n"
        "1\t5\t8\tTTT/*\t30\t30\n"
        "2\t1\t2\tA/T\t30\t30";

    const string BEDC =
        "17	7985753	7985754	T/0	-	-\n"
        "17	7985785	7985786	T/0	-	-\n"
        "17	7993250	7993257	AAAAACA/0	-	-"
        ;

    const string BEDD =
        "17	7985753	7985754	T/0	-	-\n"
        "17	7985753	7985786	TTTTTTCTCCCCCTTGAACTTGAGCTCAATTCT/0	-	-\n"
        "17	7985754	7985754	0/TTTTTCTCCCCCTTGAACTTGAGCTCAATTC	-	-\n"
        "17	7985785	7985786	T/0	-	-"
        ;

    struct MockCollector {
        bool hit(const Bed& a, const Bed& b) {
            hitsA.push_back(make_pair(a,b));
            hitsB.push_back(make_pair(b,a));
            return true;
        }
        bool wantMissA() const { return true; }
        bool wantMissB() const { return true; }
        void missA(const Bed& a) { missesA.push_back(a); }
        void missB(const Bed& b) { missesB.push_back(b); }

        vector<pair<Bed,Bed> > hitsA;
        vector<pair<Bed,Bed> > hitsB;
        vector<Bed> missesA;
        vector<Bed> missesB;
    };

    typedef boost::function<void(const BedHeader*, string&, Bed&)> Extractor;
    typedef TypedStream<Bed, Extractor> BedReader;
    Extractor extractor = boost::bind(&Bed::parseLine, _1, _2, _3, 2);
}

TEST(TestIntersectFull, intersectSelf) {
    MockCollector rc;
    stringstream ssA(BEDA);
    stringstream ssB(BEDA);
    InputStream streamA("A", ssA);
    InputStream streamB("B", ssB);
    BedReader s1(extractor, streamA);
    BedReader s2(extractor, streamB);
    IntersectFull<BedReader,BedReader,MockCollector> intersector(s1, s2, rc);
    intersector.execute();

    // each line hits twice each generating 8 total matches
    ASSERT_EQ(8u, rc.hitsA.size());
    ASSERT_EQ(8u, rc.hitsB.size());
    ASSERT_EQ(0u, rc.missesA.size());
    ASSERT_EQ(0u, rc.missesB.size());
}

TEST(TestIntersectFull, misses) {
    MockCollector rc;
    stringstream ssA(BEDA);
    stringstream ssB(BEDB);
    InputStream streamA("A", ssA);
    InputStream streamB("B", ssB);
    BedReader s1(extractor, streamA);
    BedReader s2(extractor, streamB);
    IntersectFull<BedReader,BedReader,MockCollector> intersector(s1, s2, rc);
    intersector.execute();

    // first 2 lines hit twice each, last 2 lines once each, total of 6 hits
    ASSERT_EQ(6u, rc.hitsA.size());
    ASSERT_EQ(6u, rc.hitsB.size());
    ASSERT_EQ(0u, rc.missesA.size());
    ASSERT_EQ(1u, rc.missesB.size());
}

TEST(TestIntersectFull, cacheCrash) {
    MockCollector rc;
    stringstream ssA(BEDC);
    stringstream ssB(BEDD);
    InputStream streamA("C", ssA);
    InputStream streamB("D", ssB);
    BedReader s1(extractor, streamA);
    BedReader s2(extractor, streamB);
    IntersectFull<BedReader,BedReader,MockCollector> intersector(s1, s2, rc);
    intersector.execute();

    // first 2 lines hit twice each, last 2 lines once each, total of 6 hits
    ASSERT_EQ(4u, rc.hitsA.size());
    ASSERT_EQ(4u, rc.hitsB.size());
    ASSERT_EQ(1u, rc.missesA.size());
    ASSERT_EQ(1u, rc.missesB.size());
}
