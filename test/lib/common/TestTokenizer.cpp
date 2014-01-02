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

    ASSERT_EQ(2u, t.advance(2));
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

TEST(TestTokenizer, floats) {
    string input("0.66 1e-8 44 1.2e6");
    double d;
    Tokenizer<char> t(input, ' ');
    ASSERT_TRUE(t.extract(d));
    ASSERT_NEAR(0.66, d, 1e-7);
    ASSERT_TRUE(t.extract(d));
    ASSERT_NEAR(1e-8, d, 1e-7);
    ASSERT_TRUE(t.extract(d));
    ASSERT_NEAR(44, d, 1e-7);
    ASSERT_TRUE(t.extract(d));
    ASSERT_NEAR(1.2e6, d, 1e-7);

    ASSERT_TRUE(t.eof());
    ASSERT_FALSE(t.extract(d));
}

TEST(TestTokenizer, nextTokenMatches) {
    string input("1,2,.,4");
    Tokenizer<char> t(input, ',');
    ASSERT_TRUE(t.nextTokenMatches("1"));
    t.advance();
    ASSERT_TRUE(t.nextTokenMatches("2"));
    t.advance();
    ASSERT_TRUE(t.nextTokenMatches("."));
    t.advance();
    ASSERT_TRUE(t.nextTokenMatches("4"));
    t.advance();
    ASSERT_TRUE(t.nextTokenMatches(""));
    ASSERT_TRUE(t.eof());
}

TEST(Tokenizer, extractChar) {
    string input("a,b,cd");

    Tokenizer<char> t(input, ',');
    char c(0);

    EXPECT_TRUE(t.extract(c));
    EXPECT_EQ('a', c);

    EXPECT_TRUE(t.extract(c));
    EXPECT_EQ('b', c);

    EXPECT_FALSE(t.extract(c));
}

TEST(Tokenizer, extractPointers) {
    string input("ax=11,bx=22,cx=33");
    Tokenizer<char> t(input, ',');
    char const* b(0);
    char const* e(0);

    t.extract(&b, &e);
    ASSERT_EQ("ax=11", string(b, e));

    Tokenizer<char> t2(b, e, '=');
    t2.extract(&b, &e);
    ASSERT_EQ("ax", string(b, e));
    t2.extract(&b, &e);
    ASSERT_EQ("11", string(b, e));

    t.extract(&b, &e);
    ASSERT_EQ("bx=22", string(b, e));

    t2 = Tokenizer<char>(b, e, '=');
    t2.extract(&b, &e);
    ASSERT_EQ("bx", string(b, e));
    t2.extract(&b, &e);
    ASSERT_EQ("22", string(b, e));

    t.extract(&b, &e);
    ASSERT_EQ("cx=33", string(b, e));

    t2 = Tokenizer<char>(b, e, '=');
    t2.extract(&b, &e);
    ASSERT_EQ("cx", string(b, e));
    t2.extract(&b, &e);
    ASSERT_EQ("33", string(b, e));
}

TEST(TestTokenizer, testStringView) {
    string fromDbsnp(
        "1\t13302\trs180734498\tC\tT\t.\t.\tRSPOS=13302;dbSNPBuildID=135;SSR=0;SAO=0;VP=050000000000000010000100;WGT=0;VC=SNV;KGPilot123\n"
    );
    string twice = fromDbsnp + fromDbsnp;
    StringView jxs(twice.data());
    Tokenizer<char> tok(jxs, '\n');
    StringView line;
    while (tok.extract(line)) {
        vector<StringView> fields;
        Tokenizer<char>::split(line, '\t', back_inserter(fields));
        /*
         *Tokenizer<char> t2(line, '\t');
         *StringView fld;
         *while (t2.extract(fld)) {
         *    //cout << fld << "\t";
         *    fields.push_back(fld);
         *}
         */

        for (auto f = fields.begin(); f != fields.end(); ++f)
            cout << *f << "\t";
        cout << "\n";


    }

}
