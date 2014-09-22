#include "common/String.hpp"

#include <gtest/gtest.h>

#include <string>

TEST(String, commonPrefix) {
    std::string a("abcDef");
    std::string b("abcdef");

    EXPECT_EQ(3u, commonPrefix(a, b));
}

TEST(String, commonPrefixReverseIterators) {
    std::string a("GCC");
    std::string b("GA");
    EXPECT_EQ(0u, commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));

    a="GCC";
    b="GAC";
    EXPECT_EQ(1u, commonPrefix(
        a.rbegin(), a.rend(),
        b.rbegin(), b.rend()
    ));

    a="GCC";
    b="GCC";
    EXPECT_EQ(3u, commonPrefix(
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


TEST(String, findHomopolymers) {
    std::string testSequence = "ACGTAAAACCACTGGGATT";
    HomopolymerCollector c;
    findHomopolymers(testSequence, c, 2);
    std::vector<HomopolymerCollector::Record> expected{
        {4, 8, 'A'},
        {8, 10, 'C'},
        {13, 16, 'G'},
        {17, 19, 'T'}
        };

    EXPECT_EQ(4, c.records.size());
    EXPECT_EQ(expected, c.records);

    c.records.clear();
    findHomopolymers(testSequence, c, 3);
    expected = std::vector<HomopolymerCollector::Record>{
        {4, 8, 'A'},
        {13, 16, 'G'},
        };

    EXPECT_EQ(2, c.records.size());
    EXPECT_EQ(expected, c.records);
}
