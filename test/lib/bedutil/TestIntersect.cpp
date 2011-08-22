#include "bedutil/Intersect.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/TypedStream.hpp"

#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

using namespace std;
using namespace std::placeholders;

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

    struct MockCollector {
        bool hit(const Bed& a, const Bed& b) {
            hitsA.push_back(a);
            hitsB.push_back(b);
            return true;
        }

        void missA(const Bed& a) { missesA.push_back(a); }
        void missB(const Bed& b) { missesB.push_back(b); }

        vector<Bed> hitsA;
        vector<Bed> hitsB;
        vector<Bed> missesA;
        vector<Bed> missesB;
    };

    typedef function<void(string&, Bed&)> Extractor;
    typedef TypedStream<Bed, Extractor> BedReader;
    Extractor extractor = bind(&Bed::parseLine, _1, _2, 2);
}

TEST(TestIntersect, intersectSelf) {
    MockCollector rc;
    stringstream ssA(BEDA);
    stringstream ssB(BEDA);
    InputStream streamA("A", ssA);
    InputStream streamB("B", ssB);
    BedReader s1(extractor, streamA);
    BedReader s2(extractor, streamB);
    Intersect<BedReader,BedReader,MockCollector> intersector(s1, s2, rc);
    intersector.execute();

    // each line hits twice each generating 8 total matches
    ASSERT_EQ(8, rc.hitsA.size());
    ASSERT_EQ(8, rc.hitsB.size());
    ASSERT_EQ(0, rc.missesA.size());
    ASSERT_EQ(0, rc.missesB.size());
}

TEST(TestIntersect, misses) {
    MockCollector rc;
    stringstream ssA(BEDA);
    stringstream ssB(BEDB);
    InputStream streamA("A", ssA);
    InputStream streamB("B", ssB);
    BedReader s1(extractor, streamA);
    BedReader s2(extractor, streamB);
    Intersect<BedReader,BedReader,MockCollector> intersector(s1, s2, rc);
    intersector.execute();

    // first 2 lines hit twice each, last 2 lines once each, total of 6 hits
    ASSERT_EQ(6, rc.hitsA.size());
    ASSERT_EQ(6, rc.hitsB.size());
    ASSERT_EQ(0, rc.missesA.size());
    ASSERT_EQ(1, rc.missesB.size());
}
