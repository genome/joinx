#include "common/CigarString.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

TEST(CigarString, translate) {
    ASSERT_EQ(MATCH, CigarString::translate('M'));
    ASSERT_EQ(INS, CigarString::translate('I'));
    ASSERT_EQ(DEL, CigarString::translate('D'));
    ASSERT_EQ(SKIP, CigarString::translate('N'));
    ASSERT_EQ(SOFT_CLIP, CigarString::translate('S'));
    ASSERT_EQ(HARD_CLIP, CigarString::translate('H'));
    ASSERT_EQ(PADDING, CigarString::translate('P'));
    ASSERT_EQ(SEQ_MATCH, CigarString::translate('='));
    ASSERT_EQ(SEQ_MISMATCH, CigarString::translate('X'));

    ASSERT_EQ(MATCH, CigarString::translate('m'));
    ASSERT_EQ(INS, CigarString::translate('i'));
    ASSERT_EQ(DEL, CigarString::translate('d'));
    ASSERT_EQ(SKIP, CigarString::translate('n'));
    ASSERT_EQ(SOFT_CLIP, CigarString::translate('s'));
    ASSERT_EQ(HARD_CLIP, CigarString::translate('h'));
    ASSERT_EQ(PADDING, CigarString::translate('p'));
    ASSERT_EQ(SEQ_MATCH, CigarString::translate('='));
    ASSERT_EQ(SEQ_MISMATCH, CigarString::translate('x'));

    ASSERT_EQ('M', CigarString::translate(MATCH));
    ASSERT_EQ('I', CigarString::translate(INS));
    ASSERT_EQ('D', CigarString::translate(DEL));
    ASSERT_EQ('N', CigarString::translate(SKIP));
    ASSERT_EQ('S', CigarString::translate(SOFT_CLIP));
    ASSERT_EQ('H', CigarString::translate(HARD_CLIP));
    ASSERT_EQ('P', CigarString::translate(PADDING));
    ASSERT_EQ('=', CigarString::translate(SEQ_MATCH));
    ASSERT_EQ('X', CigarString::translate(SEQ_MISMATCH));

    ASSERT_THROW(CigarString::translate('q'), runtime_error);
    ASSERT_THROW(CigarString::translate(CigarOpType(99)), runtime_error);
}

TEST(CigarString, fromString) {
    string str("99M5I99M");
    CigarString cs(str);
    CigarString::Op expected[] = {
        { 99, MATCH },
        { 5, INS },
        { 99, MATCH },
    };
    ASSERT_EQ(3u, cs.ops().size());
    for (int i = 0; i < 3; ++i)
        ASSERT_EQ(expected[i], cs.ops()[i]) << "failed at i=" << i;
    ASSERT_EQ(str, string(cs));
}

TEST(CigarString, push_back) {
    CigarString::Op expected[] = {
        { 99, MATCH },
        { 5, INS },
        { 99, MATCH },
    };

    CigarString cs;
    ASSERT_EQ("", string(cs));
    for (int i = 0; i < 3; ++i)
        cs.push_back(expected[i]);
    ASSERT_EQ("99M5I99M", string(cs));
}

TEST(CigarString, pop_front) {
    CigarString c("10M10I10D10S");
    c.pop_front(5);
    ASSERT_EQ("5M10I10D10S", string(c));
    c.pop_front(6);
    ASSERT_EQ("9I10D10S", string(c));
    c.pop_front(13);
    ASSERT_EQ("6D10S", string(c));
    c.pop_front(7);
    ASSERT_EQ("9S", string(c));
    c.pop_front(50);
    ASSERT_EQ("", string(c));
}

TEST(CigarString, subset) {
    CigarString c("10M10I10M10D1M");
    ASSERT_EQ("10M2I", string(c.subset(0, 12)));
    ASSERT_EQ("10M10I1M", string(c.subset(0, 21)));
    ASSERT_EQ("10M10I1M", string(c.subset(0, 21)));
    ASSERT_EQ("1M10I1M", string(c.subset(9, 12)));
    ASSERT_EQ("1I10M10D1M", string(c.subset(19, 12)));
    ASSERT_EQ("10M", string(c.subset(20, 10)));
    ASSERT_EQ("1M", string(c.subset(30, 1)));
    ASSERT_EQ("1M", string(c.subset(29, 1)));

    ASSERT_EQ("10I", string(c.subset(10, 10)));
    ASSERT_EQ("", string(c.subset(39, 1)));
    ASSERT_EQ("", string(c.subset(3, 0)));
    ASSERT_EQ("", string(c.subset(30, 0)));
}

TEST(CigarString, merge) {
    CigarString a("99M5I99M");
    CigarString b("100M");

    ASSERT_EQ("99M1I", string(CigarString::merge(a,b,0)));

    b = "8M1X91M";
    ASSERT_EQ("8M1X90M1I", string(CigarString::merge(a,b,0)));

    a = "10M5D10M";
    b = "20M";
    ASSERT_EQ("10M5D10M", string(CigarString::merge(a,b,0)));
    b = "2M";
    ASSERT_EQ("1M5D1M", string(CigarString::merge(a,b,9)));

    a = "99M10D99M";
    b = "99M3I99M";
    ASSERT_EQ("99M3I10D99M", string(CigarString::merge(a,b,0)));
}

TEST(CigarString, structural) {
    CigarString c("99=1X99=");
    ASSERT_EQ("199M", string(c.structural()));

    c = "99=3I99=";
    ASSERT_EQ("99M3I99M", string(c.structural()));

    c = "99=3D99=";
    ASSERT_EQ("99M3D99M", string(c.structural()));

    c = "99S3D99=";
    ASSERT_EQ("99S3D99M", string(c.structural()));
}
