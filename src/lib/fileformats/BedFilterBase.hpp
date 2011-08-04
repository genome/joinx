#pragma once

#include <cstdint>

class Bed;

class BedFilterBase {
public:
    BedFilterBase() : _filtered(0) {}
    ~BedFilterBase() {}

    uint64_t filtered() {
        return _filtered;
    }

    bool exclude(const Bed& snv) {
        if (_exclude(snv)) {
            ++_filtered;
            return true;
        }
        return false;
    }

protected:
    virtual bool _exclude(const Bed& snv) = 0;

protected:
    uint64_t _filtered;
};
