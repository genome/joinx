#pragma once

#include "common/CoordinateView.hpp"
#include "common/Region.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

template<
          typename ReaderType
        , typename OutputFunc
        , typename CoordView = DefaultCoordinateView
        >
class GroupOverlapping {
public:
    typedef typename ReaderType::ValueType ValueType;
    typedef std::vector<std::unique_ptr<ValueType>> OutputType;

    GroupOverlapping(ReaderType& reader, OutputFunc& out, CoordView coordView = CoordView())
        : reader_(reader)
        , out_(out)
        , coordView_(coordView)
    {
    }

    void push_entry(std::unique_ptr<ValueType> entry) {
        if (!overlaps(*entry)) {
            assignRegion(*entry);
            if (!bundle_.empty()) {
                out_(std::move(bundle_));
                bundle_.clear();
            }
        }
        region_.end = std::max(coordView_.stop(*entry), region_.end);
        bundle_.push_back(std::move(entry));
    }

    void flush() {
        if (!bundle_.empty()) {
            out_(std::move(bundle_));
            bundle_.clear();
        }
    }

    void execute() {
        std::unique_ptr<ValueType> entry(new ValueType);
        while (reader_.next(*entry)) {
            push_entry(std::move(entry));
            entry.reset(new ValueType);
        }
        flush();
    }

private:
    bool overlaps(ValueType const& entry) {
        if (!sequence_.empty() && coordView_.chrom(entry) == sequence_) {
            Region r{coordView_.start(entry), coordView_.stop(entry)};
            return region_.overlap(r) > 0;
        }
        return false;
    }

    void assignRegion(ValueType const& entry) {
        sequence_ = coordView_.chrom(entry);
        region_.begin = coordView_.start(entry);
        region_.end = coordView_.stop(entry);
    }

private:
    ReaderType& reader_;
    OutputFunc& out_;

    std::string sequence_;
    Region region_;
    CoordView coordView_;
    OutputType bundle_;
};


template<typename ReaderType, typename OutputFunc>
GroupOverlapping<ReaderType, OutputFunc>
makeGroupOverlapping(ReaderType& reader, OutputFunc& out) {
    return GroupOverlapping<ReaderType, OutputFunc>(reader, out);
}
