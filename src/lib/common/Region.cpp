#include "Region.hpp"

#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <string>

using boost::lexical_cast;
using namespace std;

namespace {
    bool lessThan(int64_t pos1, int64_t pos2) { return pos1 < pos2; }
    bool greaterThan(int64_t pos1, int64_t pos2) { return pos1 > pos2; }
}

Region::Region()
    : _strand(0)
    , _start(0)
    , _stop(0)
    , _strandedStart(0)
    , _strandedStop(0)
    , strandedLess(&lessThan)
{}

Region::Region(int strand, int64_t start, int64_t stop)
    : _strand(strand)
    , _start(start)
    , _stop(stop)
    , _strandedStart(strand == 1 ? start : stop)
    , _strandedStop(strand == 1 ? stop : start)
    , strandedLess(strand == 1 ? &lessThan : &greaterThan)
{
    if (strand != 1 && strand != -1)
        throw runtime_error("Invalid strand: " + lexical_cast<string>(strand));
}


Region::RelativePos Region::distance(int64_t pos) const {
    if (pos >= _start && pos <= _stop) {
        return RelativePos(RelativePos::IN, 0);
    } else if (strandedLess(pos, _strandedStart)) {
        return RelativePos(RelativePos::BEFORE, abs(pos - _strandedStart));
    } else if (strandedLess(_strandedStop, pos)) {
        return RelativePos(RelativePos::AFTER, abs(pos - _strandedStop));
    }

    return RelativePos();
}

int64_t Region::distanceFromStart(int64_t pos) const {
    return abs(pos-_strandedStart);
}

int64_t Region::distanceFromStop(int64_t pos) const {
    return abs(pos-_strandedStop);
}

