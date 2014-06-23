#include "common/Sequence.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

using namespace std;

TEST(Sequence, reverseComplement) {
    string input = "acgtrymkswhbvdnx";
    string expected = "XNHBVDWSMKRYACGT";

    Sequence seq(input);
    ASSERT_EQ(expected, seq.reverseComplementData());
}

TEST(Sequence, fromStream) {
    stringstream ss("ACGTACGTACGT");
    string expected = "ACGTA";
    Sequence seq(ss, 5);
    ASSERT_EQ(expected, seq.data());
}

TEST(Sequence, null) {
    Sequence star("*");
    ASSERT_TRUE(star.null());

    Sequence zero("0");
    ASSERT_TRUE(zero.null());

    Sequence dash("-");
    ASSERT_TRUE(dash.null());
}

TEST(Sequence, commonPrefixReverseIterators) {
    string a("GCC");
    string b("GA");
    ASSERT_EQ(0u, Sequence::commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));

    a="GCC";
    b="GAC";
    ASSERT_EQ(1u, Sequence::commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));

    a="GCC";
    b="GCC";
    ASSERT_EQ(3u, Sequence::commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));
}

struct HomopolymerCollector {
    struct Record {
        size_t start;
        size_t stop;
        char base;

        bool operator==(Record const& rhs) const {
            return start == rhs.start && stop == rhs.stop && base == rhs.base;
        }

        friend std::ostream& operator<<(std::ostream& s, Record const& rec) {
            s << rec.start << "," << rec.stop << "," << rec.base;
            return s;
        }
    };

    void operator()(size_t start, size_t stop, char base) {
        records.emplace_back(Record{start, stop, base});
    }

    std::vector<Record> records;
};


TEST(Sequence, findHomopolymers) {
    std::string testSequence = "ACGTAAAACCACTGGGATT";
    HomopolymerCollector c;
    Sequence::findHomopolymers(testSequence, c, 2);
    std::vector<HomopolymerCollector::Record> expected{
        {4, 8, 'A'},
        {8, 10, 'C'},
        {13, 16, 'G'},
        {17, 19, 'T'}
        };

    EXPECT_EQ(4, c.records.size());
    EXPECT_EQ(expected, c.records);

    c.records.clear();
    Sequence::findHomopolymers(testSequence, c, 3);
    expected = std::vector<HomopolymerCollector::Record>{
        {4, 8, 'A'},
        {13, 16, 'G'},
        };

    EXPECT_EQ(2, c.records.size());
    EXPECT_EQ(expected, c.records);
}
