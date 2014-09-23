#include "common/String.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(TestString, commonPrefixMulti_all_empty) {
    std::string a("");
    std::string b("");
    std::string c("");
    std::string d("");

    std::vector<std::string> xs{a, b, c, d};
    EXPECT_EQ(0u, commonPrefixMulti(xs));
    EXPECT_EQ(0u, commonSuffixMulti(xs));
}

TEST(TestString, commonPrefixMulti_no_strings) {
    std::vector<std::string> xs;
    EXPECT_EQ(0u, commonPrefixMulti(xs));
    EXPECT_EQ(0u, commonSuffixMulti(xs));
}

TEST(TestString, commonPrefixMulti) {
    std::string a("abcdefGh");
    std::string b("abcdeFGh");
    std::string c("abcdEfGh");
    std::string d("abcDefGh");

    std::vector<std::string> xs{a, b, c, d};
    EXPECT_EQ(3u, commonPrefixMulti(xs));
    EXPECT_EQ(2u, commonSuffixMulti(xs));
}

TEST(TestString, commonPrefixMultiEmpty) {
    std::string a("abcdefGh");
    std::string b("abcdeFGh");
    std::string c("abcdEfGh");
    std::string d("");

    std::vector<std::string> xs{a, b, c, d};
    EXPECT_EQ(0u, commonPrefixMulti(xs));
    EXPECT_EQ(0u, commonSuffixMulti(xs));
}

TEST(TestString, commonPrefix) {
    std::string a("abcDef");
    std::string b("abcdef");

    EXPECT_EQ(3u, commonPrefix(a, b));
    EXPECT_EQ(a.size(), commonPrefix(a, a));
    EXPECT_EQ(b.size(), commonPrefix(b, b));
}

TEST(TestString, commonPrefixEmpty) {
    std::string a("abcDef");
    std::string b("");

    EXPECT_EQ(0u, commonPrefix(a, b));
    EXPECT_EQ(0u, commonPrefix(b, b));
}

TEST(TestString, commonPrefixReverseIterators) {
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


TEST(TestString, findHomopolymers) {
    std::string testSequence = "ACGTAAAACCACTGGGATT";
    HomopolymerCollector c;
    findHomopolymers(testSequence, c, 2);
    std::vector<HomopolymerCollector::Record> expected{
        {4, 8, 'A'},
        {8, 10, 'C'},
        {13, 16, 'G'},
        {17, 19, 'T'}
        };

    EXPECT_EQ(4u, c.records.size());
    EXPECT_EQ(expected, c.records);

    c.records.clear();
    findHomopolymers(testSequence, c, 3);
    expected = std::vector<HomopolymerCollector::Record>{
        {4, 8, 'A'},
        {13, 16, 'G'},
        };

    EXPECT_EQ(2u, c.records.size());
    EXPECT_EQ(expected, c.records);
}
