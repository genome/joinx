#pragma once

#include "common/Region.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

template<typename ReaderType, typename OutputFunc>
class GroupOverlapping {
public:
    typedef typename ReaderType::ValueType ValueType;
    typedef std::vector<std::unique_ptr<ValueType>> OutputType;

    GroupOverlapping(ReaderType& reader, OutputFunc& out)
        : reader_(reader)
        , out_(out)
    {
    }

    void execute() {
        std::unique_ptr<ValueType> entry(new ValueType);
        OutputType entries;
        while (reader_.next(*entry)) {
            if (!overlaps(*entry)) {
                assignRegion(*entry);
                if (!entries.empty()) {
                    out_(std::move(entries));
                    entries.clear();
                }
            }
            region_.end = std::max(entry->stop(), region_.end);
            entries.push_back(std::move(entry));
            entry.reset(new ValueType);
        }

        out_(std::move(entries));
    }

private:
    bool overlaps(ValueType const& entry) {
        if (!sequence_.empty() && entry.chrom() == sequence_) {
            Region r{entry.start(), entry.stop()};
            return region_.overlap(r) > 0;
        }
        return false;
    }

    void assignRegion(ValueType const& entry) {
        sequence_ = entry.chrom();
        region_.begin = entry.start();
        region_.end = entry.stop();
    }

private:
    ReaderType& reader_;
    OutputFunc& out_;

    std::string sequence_;
    Region region_;
};


template<typename ReaderType, typename OutputFunc>
GroupOverlapping<ReaderType, OutputFunc>
makeGroupOverlapping(ReaderType& reader, OutputFunc& out) {
    return GroupOverlapping<ReaderType, OutputFunc>(reader, out);
}
