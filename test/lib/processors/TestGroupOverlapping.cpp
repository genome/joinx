#include "processors/GroupOverlapping.hpp"

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
        os << me.chrom() << "\t" << me.start() << "\t" << me.stop() << "\t" << me.source();
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

TEST(TestGroupOverlapping, read) {
    std::deque<MockEntry> entries{
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

    MockReader reader{entries};

    Collector collector;

    auto oer = makeGroupOverlapping(reader, collector);
    oer.execute();

    auto const& result = collector.entries;
    EXPECT_EQ(4, result.size()); // should get all four groups in output

    // Check first group
    ASSERT_EQ(3, result[0].size());
    EXPECT_EQ(entries[0], *result[0][0]);
    EXPECT_EQ(entries[1], *result[0][1]);
    EXPECT_EQ(entries[2], *result[0][2]);

    // Second group
    ASSERT_EQ(1, result[1].size());
    EXPECT_EQ(entries[3], *result[1][0]);

    // Third group
    ASSERT_EQ(1, result[2].size());
    EXPECT_EQ(entries[4], *result[2][0]);

    // Fourth group
    ASSERT_EQ(2, result[3].size());
    EXPECT_EQ(entries[5], *result[3][0]);
    EXPECT_EQ(entries[6], *result[3][1]);
}
