#include "fileformats/InputStream.hpp"

#include <sstream>
#include <gtest/gtest.h>

using namespace std;

TEST(InputStream, caching) {
    stringstream ss("1\n2\n3\n");
    string line;
    InputStream stream("test", ss);
    stream.caching(true);

    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("1", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("2", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("3", line);
    ASSERT_FALSE(stream.getline(line));
    ASSERT_TRUE(stream.eof());

    stream.caching(false);
    ASSERT_FALSE(stream.eof());
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("1", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("2", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("3", line);
    ASSERT_FALSE(stream.getline(line));
    ASSERT_TRUE(stream.eof());
}
