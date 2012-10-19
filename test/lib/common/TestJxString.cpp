#include "common/JxString.hpp"

#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

TEST(TestJxString, construct) {
    string input("testing this silly object");
    JxString s1(input.data());
    ASSERT_EQ(input.size(), s1.size());
    ASSERT_EQ(input, s1);
    ASSERT_EQ(input.data(), s1);
    ASSERT_TRUE(s1.startsWith(input));
    ASSERT_TRUE(s1.startsWith("test"));
    ASSERT_FALSE(s1.startsWith("banana"));

    s1.assign(input.data(), input.data()+4);

    ASSERT_EQ("test", s1);

    stringstream ss;
    ss << s1;
    ASSERT_EQ("test", ss.str());

    s1.clear();
    ASSERT_TRUE(s1.empty());
    ASSERT_EQ(0, s1.size());
    ASSERT_EQ("", s1);
}

TEST(TestJxString, startsWith) {
    string _comment("#comment");
    string _data("not a comment");
    JxString comment(_comment.data());
    JxString data(_data.data());
    ASSERT_FALSE(data.startsWith("#"));
    ASSERT_TRUE(comment.startsWith("#"));
}

TEST(TestJxString, equal) {
    string _data("test");
    JxString data(_data.data());
    ASSERT_EQ(data, "test");
    ASSERT_FALSE(data == "testf");
    ASSERT_FALSE(data == "tesf");
    ASSERT_FALSE(data == "tes");
}
