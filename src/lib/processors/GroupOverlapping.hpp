#pragma once

#include "common/CoordinateView.hpp"
#include "common/LocusCompare.hpp"
#include "common/Region.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

template<
          typename ValueType
        , typename OutputFunc
        , typename Compare
        >
class GroupSorter {
public:
    GroupSorter(OutputFunc& out, Compare cmp = Compare())
        : out(out)
        , cmp(cmp)
    {}

    typedef std::unique_ptr<ValueType> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    void operator()(ValuePtrVector entries) {
        // OMG gcc4.4 stdlibc++ you are going to make me die
        // Why can't you sort a container of unique_ptrs??!
        DerefCompare<Compare> dcmp(cmp);

// FIXME: ifdef this only for old compilers that can't sort unique_ptrs
        std::vector<ValueType*> rawPtrs(entries.size());
        for (std::size_t i = 0; i < entries.size(); ++i) {
            rawPtrs[i] = entries[i].release();
        }
        std::sort(rawPtrs.begin(), rawPtrs.end(), dcmp);
        for (std::size_t i = 0; i < entries.size(); ++i) {
            entries[i].reset(rawPtrs[i]);
        }

        out(std::move(entries));
    }

    OutputFunc& out;
    Compare cmp;
};

template<
          typename ValueType
        , typename OutputFunc
        , typename Compare
        >
GroupSorter<ValueType, OutputFunc, Compare>
makeGroupSorter(OutputFunc& out, Compare cmp = Compare()) {
    return GroupSorter<ValueType, OutputFunc, Compare>(out, cmp);
}


template<
          typename ValueType
        , typename OutputFunc
        , typename CoordView = DefaultCoordinateView
        >
class GroupOverlapping {
public:
    typedef std::unique_ptr<ValueType> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    GroupOverlapping(OutputFunc& out, CoordView coordView = CoordView())
        : out_(out)
        , coordView_(coordView)
    {
    }

    // GCC 4.4 requires writing these explicitly :(
    GroupOverlapping(GroupOverlapping const&) = delete;
    GroupOverlapping& operator=(GroupOverlapping const&) = delete;
    GroupOverlapping(GroupOverlapping&& rhs)
        : out_(rhs.out_)
        , sequence_(std::move(rhs.sequence_))
        , region_(std::move(rhs.region_))
        , coordView_(std::move(rhs.coordView_))
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

    std::string sequence_;
    Region region_;
    CoordView coordView_;
    ValuePtrVector bundle_;
};


template<
          typename ValueType
        , typename OutputFunc
        , typename CoordView = DefaultCoordinateView
        >
GroupOverlapping<ValueType, OutputFunc, CoordView>
makeGroupOverlapping(OutputFunc& out, CoordView coordView = CoordView()) {
    return GroupOverlapping<ValueType, OutputFunc, CoordView>(out, coordView);
}
