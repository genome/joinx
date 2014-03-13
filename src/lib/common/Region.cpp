#include "Region.hpp"

int64_t Region::overlap(Region const& that) const {
    if (that.end <= begin || that.begin >= end)
        return 0;

    return std::min(end, that.end) - std::max(begin, that.begin);
}

bool Region::contains(Region const& that) const {
    return that.begin >= begin && that.end <= end;
}

bool Region::isContainedIn(Region const& that) const {
    return that.contains(*this);
}

int64_t Region::size() const {
    return end - begin;
}
