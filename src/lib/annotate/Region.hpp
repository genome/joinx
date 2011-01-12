#pragma once

#include "bedutil/intconfig.hpp"

class Region {
public:
    struct RelativePos {
        enum Type {
            ERROR,
            BEFORE,
            IN,
            AFTER
        };

        RelativePos() : type(ERROR), dist(0) {}
        RelativePos(Type t, int64_t d) : type(t), dist(d) {}
        
        Type type;
        int64_t dist;
    };

    Region();
    Region(int strand, int64_t start, int64_t stop);

    int strand() const;
    int64_t start() const;
    int64_t stop() const;
    int64_t strandedStart() const;
    int64_t strandedStop() const;

    RelativePos distance(int64_t pos) const;

protected:
    int _strand;
    int64_t _start;
    int64_t _stop;

    int64_t _strandedStart;
    int64_t _strandedStop;

private:
    bool (*strandedLess)(int64_t, int64_t);
};

inline int Region::strand() const {
    return _strand;
}

inline int64_t Region::start() const {
    return _start;
}

inline int64_t Region::stop() const {
    return _stop;
}

inline int64_t Region::strandedStart() const {
    return _strandedStart;
}

inline int64_t Region::strandedStop() const {
    return _strandedStop;
}
