#pragma once

#include "common/CoordinateView.hpp"
#include "common/LocusCompare.hpp"
#include "common/Region.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
    void nothing() {}
}

template<
          typename ValueType
        , typename OutputFunc
        , typename CoordView = DefaultCoordinateView
        , typename BeginFunc = void(*)()
        , typename EndFunc = void(*)()
        >
class GroupOverlapping {
public:
    typedef std::unique_ptr<ValueType> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    GroupOverlapping(
              OutputFunc& out
            , CoordView coordView = CoordView()
            , BeginFunc beginFunc = nothing
            , EndFunc endFunc = nothing
            )
        : out_(out)
        , coordView_(coordView)
        , beginFunc_(beginFunc)
        , endFunc_(endFunc)
    {
    }

    // GCC 4.4 requires writing these explicitly :(
    GroupOverlapping(GroupOverlapping const&) = delete;
    GroupOverlapping& operator=(GroupOverlapping const&) = delete;
    GroupOverlapping(GroupOverlapping&& rhs)
        : out_(rhs.out_)
        , coordView_(std::move(rhs.coordView_))
        , beginFunc_(std::move(rhs.beginFunc_))
        , endFunc_(std::move(rhs.endFunc_))
        , sequence_(std::move(rhs.sequence_))
        , region_(std::move(rhs.region_))
        , bundle_(std::move(rhs.bundle_))
    {}

    void operator()(ValuePtrVector entries) {
        for (auto i = entries.begin(); i != entries.end(); ++i)
            (*this)(std::move(*i));
    }

    void operator()(ValuePtr entry) {
        if (!overlaps(*entry)) {
            assignRegion(*entry);
            if (!bundle_.empty()) {
                beginFunc_();
                out_(std::move(bundle_));
                endFunc_();
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
    OutputFunc& out_;
    CoordView coordView_;
    BeginFunc beginFunc_;
    EndFunc endFunc_;

    std::string sequence_;
    Region region_;
    ValuePtrVector bundle_;
};


template<
          typename ValueType
        , typename OutputFunc
        , typename CoordView = DefaultCoordinateView
        , typename BeginFunc = void(*)()
        , typename EndFunc = void(*)()
        >
GroupOverlapping<ValueType, OutputFunc, CoordView, BeginFunc, EndFunc>
makeGroupOverlapping(
          OutputFunc& out
        , CoordView coordView = CoordView()
        , BeginFunc beginFunc = nothing
        , EndFunc endFunc = nothing
        )
{
    return GroupOverlapping<ValueType, OutputFunc, CoordView, BeginFunc, EndFunc>(
        out, coordView, beginFunc, endFunc
        );
}
