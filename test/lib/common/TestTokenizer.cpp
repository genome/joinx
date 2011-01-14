#include "common/Tokenizer.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;


TEST(TestTokenizer, extract) {
    string input("123\tnot\t-456");

    Tokenizer t(input);
    uint32_t unsignedValue;
    int32_t signedValue;
    string stringValue;

    ASSERT_TRUE(t.extractUnsigned(unsignedValue));
    ASSERT_EQ(123u, unsignedValue);

    ASSERT_FALSE(t.extractUnsigned(unsignedValue));
    ASSERT_FALSE(t.extractSigned(signedValue));

    ASSERT_EQ(3, t.extractString(stringValue));
    ASSERT_EQ("not", stringValue);

    ASSERT_TRUE(t.extractSigned(signedValue));
    ASSERT_EQ(-456, signedValue);
}

TEST(TestTokenizer, rewind) {
    string input("1\t2\t3");
    int n;
    Tokenizer t(input);

    for (int i = 1; i <=3; ++i) {
        ASSERT_TRUE(t.extractSigned(n));
        ASSERT_EQ(i, n);
    }

    t.rewind();

    for (int i = 1; i <=3; ++i) {
        ASSERT_TRUE(t.extractSigned(n));
        ASSERT_EQ(i, n);
    }
}

TEST(TestTokenizer, advance) {
    string input("1\t2\t3\t4\tfive");

    Tokenizer t(input);
    int n;
    string s;

    ASSERT_TRUE(t.advance());
    ASSERT_TRUE(t.extractSigned(n));
    ASSERT_EQ(2, n);

    ASSERT_EQ(2, t.advance(2));
    ASSERT_EQ(4, t.extractString(s)) << "got " << s;
    ASSERT_EQ("five", s);
}
