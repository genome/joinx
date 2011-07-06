#include "common/Tokenizer.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(TestTokenizer, stringDelim) {
    string input("1,2-5,7");
    Tokenizer<string> t(input, "-,");

    string s;

    ASSERT_TRUE(t.extract(s));
    ASSERT_EQ("1", s);
    ASSERT_EQ(',', t.lastDelim());

    ASSERT_TRUE(t.extract(s));
    ASSERT_EQ("2", s);
    ASSERT_EQ('-', t.lastDelim());

    ASSERT_TRUE(t.extract(s));
    ASSERT_EQ("5", s);
    ASSERT_EQ(',', t.lastDelim());

    ASSERT_TRUE(t.extract(s));
    ASSERT_EQ("7", s);
    ASSERT_EQ('\0', t.lastDelim());

    ASSERT_TRUE(t.eof());
    ASSERT_FALSE(t.extract(s));
}

TEST(TestTokenizer, extractSingleToken) {
    string input("hi");
    string hi;

    Tokenizer<char> t(input);
    ASSERT_FALSE(t.eof());

    ASSERT_TRUE(t.extract(hi));
    ASSERT_EQ("hi", hi);

    ASSERT_FALSE(t.extract(hi));
}

TEST(TestTokenizer, extract) {
    string input("123\tnot\t-456");
    uint32_t unsignedValue;
    int32_t signedValue;
    string stringValue;

    Tokenizer<char> t(input);
    ASSERT_FALSE(t.eof());

    ASSERT_TRUE(t.extract(unsignedValue));
    ASSERT_EQ(123u, unsignedValue);

    ASSERT_FALSE(t.extract(unsignedValue));
    ASSERT_FALSE(t.extract(signedValue));

    ASSERT_TRUE(t.extract(stringValue));
    ASSERT_EQ("not", stringValue);

    ASSERT_TRUE(t.extract(signedValue));
    ASSERT_EQ(-456, signedValue);
}

TEST(TestTokenizer, rewind) {
    string input("1\t2\t3");
    int n;

    Tokenizer<char> t(input);
    ASSERT_FALSE(t.eof());

    for (int i = 1; i <=3; ++i) {
        ASSERT_TRUE(t.extract(n));
        ASSERT_EQ(i, n);
    }

    t.rewind();

    for (int i = 1; i <=3; ++i) {
        ASSERT_TRUE(t.extract(n));
        ASSERT_EQ(i, n);
    }
}

TEST(TestTokenizer, advance) {
    string input("1\t2\t3\t4\tfive");
    int n;
    string s;

    Tokenizer<char> t(input);
    ASSERT_FALSE(t.eof());

    ASSERT_TRUE(t.advance());
    ASSERT_TRUE(t.extract(n));
    ASSERT_EQ(2, n);

    ASSERT_EQ(2, t.advance(2));
    ASSERT_TRUE(t.extract(s));
    ASSERT_EQ("five", s);
    ASSERT_TRUE(t.eof());
}

TEST(TestTokenizer, nullFields) {
    string input(",1,,3,");
    string s;

    Tokenizer<char> t(input, ',');
    ASSERT_FALSE(t.eof());

    string expected[5] = { "", "1", "", "3", "" };
    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(t.extract(s));
        ASSERT_EQ(expected[i], s);
        ASSERT_FALSE(t.eof());
    }
    t.extract(s);
    ASSERT_EQ("", s);
    ASSERT_TRUE(t.eof());
}

TEST(TestTokenizer, eof) {
    string input("1,2,3");
    int n;
    Tokenizer<char> t(input, ',');

    for (int i = 0; i < 3; ++i) {
        ASSERT_FALSE(t.eof());
        ASSERT_TRUE(t.extract(n));
    }
    
    ASSERT_TRUE(t.eof());
    ASSERT_FALSE(t.extract(n));
    ASSERT_TRUE(t.eof());
}

TEST(TestTokenizer, remaining) {
    string input("1,2,3");
    Tokenizer<char> t(input, ',');
    string s;

    t.remaining(s); 
    ASSERT_EQ("1,2,3", s);
    ASSERT_TRUE(t.extract(s));
    t.remaining(s); 
    ASSERT_EQ("2,3", s);
    ASSERT_TRUE(t.extract(s));
    t.remaining(s); 
    ASSERT_EQ("3", s);
    ASSERT_TRUE(t.extract(s));
    t.remaining(s); 
    ASSERT_EQ("", s);
    t.remaining(s); 
    ASSERT_EQ("", s);
}
