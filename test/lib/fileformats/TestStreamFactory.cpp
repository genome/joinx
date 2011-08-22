#include "fileformats/StreamFactory.hpp"

#include <gtest/gtest.h>
#include <string>
#include <functional>

using namespace std;
using namespace std::placeholders;

struct Foo {
    static void parseLine(string& s, Foo& value) {
        value.line = s;
    }

    void swap(Foo& other) {
        line.swap(other.line);
    }

    string line;
};

TEST(StreamFactory, works) {
    typedef std::function<void(string&, Foo&)> Extractor;
    typedef StreamFactory<Foo, Extractor> FactoryType;
    Extractor e(bind(&Foo::parseLine, _1, _2));
    FactoryType factory(e);
    stringstream ss1("test1\n");
    stringstream ss2("test2\n");
    InputStream is1("1", ss1);
    InputStream is2("2", ss2);
    Foo* vp1;
    Foo* vp2;
    FactoryType::StreamPtr p1 = factory.open(is1);
    FactoryType::StreamPtr p2 = factory.open(is2);
    ASSERT_TRUE(p1->peek(&vp1));
    ASSERT_TRUE(p2->peek(&vp2));
    ASSERT_EQ("test1", vp1->line);
    ASSERT_EQ("test2", vp2->line);
}
