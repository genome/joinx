#pragma once

#include <ctime>

// measure elapsed cpu time
class Timer {
public:
    Timer()
        : _start(clock())
    {
    }

    double elapsed() const {
        return (clock() - _start) / double(CLOCKS_PER_SEC);
    }

    void reset() {
        _start = clock();
    }

protected:
    clock_t _start;
};
