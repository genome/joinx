#include "processors/grouping/GroupOverlapping.hpp"
#include "processors/grouping/GroupSorter.hpp"

#include "fileformats/StreamPump.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <deque>
#include <utility>
#include <iostream>

namespace {
    struct MockEntry {
        std::string const& chrom() const { return chrom_; }
        int64_t start() const { return start_; }
        int64_t stop() const { return stop_; }
        uint32_t source() const { return source_; }

        int64_t startWithoutPadding() const { return -start_; }
        int64_t stopWithoutPadding() const { return -stop_; }

        bool operator==(MockEntry const& rhs) const {
            return chrom_ == rhs.chrom_ &&
                start_ == rhs.start_ &&
                stop_ == rhs.stop_ &&
                source_ == rhs.source_;
        }

        std::string chrom_;
        int64_t start_;
        int64_t stop_;
        uint32_t source_;
    };

    std::ostream& operator<<(std::ostream& os, MockEntry const& me) {
        os << "[" << me.chrom() << "\t" << me.start() << "\t"
            << me.stop() << "\t" << me.source() << "]";
        return os;
    }

    struct MockReader {
        typedef MockEntry ValueType;

        bool next(ValueType& value) {
            if (entries.empty())
                return false;

            value = entries.front();
            entries.pop_front();
            return true;
        }

        std::deque<ValueType> entries;
    };

    struct Collector {
        typedef std::vector<std::unique_ptr<MockEntry>> EntryList;

        void operator()(EntryList&& ents) {
            entries.push_back(std::move(ents));
        }

        std::vector<EntryList> entries;
    };
}

class TestGroupOverlapping : public ::testing::Test {
public:

    void SetUp() {
        entries = {
            // First group {chrom, start, stop, file index}
            {"1", 10, 15, 0},
            {"1", 14, 17, 1},
            {"1", 16, 19, 2},

            // Second
            {"1", 20, 22, 1},

            // Third
            {"2", 20, 22, 0},

            // Fourth
            {"2", 50, 60, 1},
            {"2", 55, 61, 2},
            };
    }

protected:
    std::deque<MockEntry> entries;
};

TEST_F(TestGroupOverlapping, read) {
    MockReader reader{entries};

    Collector collector;

    auto oer = makeGroupOverlapping<MockEntry>(collector);
    auto pump = makePointerStreamPump(reader, oer);
    pump.execute();

    auto const& result = collector.entries;
    ASSERT_EQ(4u, result.size()); // should get all four groups in output

    // Check first group
    ASSERT_EQ(3u, result[0].size());
    EXPECT_EQ(entries[0], *result[0][0]);
    EXPECT_EQ(entries[1], *result[0][1]);
    EXPECT_EQ(entries[2], *result[0][2]);

    // Second group
    ASSERT_EQ(1u, result[1].size());
    EXPECT_EQ(entries[3], *result[1][0]);

    // Third group
    ASSERT_EQ(1u, result[2].size());
    EXPECT_EQ(entries[4], *result[2][0]);

    // Fourth group
    ASSERT_EQ(2u, result[3].size());
    EXPECT_EQ(entries[5], *result[3][0]);
    EXPECT_EQ(entries[6], *result[3][1]);
}


TEST_F(TestGroupOverlapping, accept_vector) {
    std::vector<std::unique_ptr<MockEntry>> entry_ptrs;
    for (auto i = entries.begin(); i != entries.end(); ++i) {
        entry_ptrs.emplace_back(new MockEntry(*i));
    }

    Collector collector;

    auto oer = makeGroupOverlapping<MockEntry>(collector);
    oer(std::move(entry_ptrs));
    oer.flush();

    auto const& xs = collector.entries;
    ASSERT_EQ(4u, xs.size()); // should get all four groups in output

    // Check first group
    ASSERT_EQ(3u, xs[0].size());
    EXPECT_EQ(entries[0], *xs[0][0]);
    EXPECT_EQ(entries[1], *xs[0][1]);
    EXPECT_EQ(entries[2], *xs[0][2]);

    // Second group
    ASSERT_EQ(1u, xs[1].size());
    EXPECT_EQ(entries[3], *xs[1][0]);

    // Third group
    ASSERT_EQ(1u, xs[2].size());
    EXPECT_EQ(entries[4], *xs[2][0]);

    // Fourth group
    ASSERT_EQ(2u, xs[3].size());
    EXPECT_EQ(entries[5], *xs[3][0]);
    EXPECT_EQ(entries[6], *xs[3][1]);
}

TEST_F(TestGroupOverlapping, sort) {
    MockReader reader{entries};

    Collector collector;
    CompareToGreaterThan<LocusCompare<>> cmp;

    auto sorter = makeGroupSorter<MockEntry>(collector, cmp);
    auto oer = makeGroupOverlapping<MockEntry>(sorter);
    auto pump = makePointerStreamPump(reader, oer);
    pump.execute();

    auto const& xs = collector.entries;
    for (auto i = xs.begin(); i != xs.end(); ++i) {
        std::cout << "<===== begin group =====>\n";
        for (auto j = i->begin(); j != i->end(); ++j) {
            std::cout << **j << "\n";
        }
        std::cout << "<=====  end group  =====>\n";
    }

    // Each group should now be sorted largest to smallest
    // Check first group
    ASSERT_EQ(3u, xs[0].size());
    EXPECT_EQ(entries[2], *xs[0][0]);
    EXPECT_EQ(entries[1], *xs[0][1]);
    EXPECT_EQ(entries[0], *xs[0][2]);

    // Second group
    ASSERT_EQ(1u, xs[1].size());
    EXPECT_EQ(entries[3], *xs[1][0]);

    // Third group
    ASSERT_EQ(1u, xs[2].size());
    EXPECT_EQ(entries[4], *xs[2][0]);

    // Fourth group
    ASSERT_EQ(2u, xs[3].size());
    EXPECT_EQ(entries[6], *xs[3][0]);
    EXPECT_EQ(entries[5], *xs[3][1]);
}
