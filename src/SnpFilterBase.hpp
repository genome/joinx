#pragma once

#include "intconfig.hpp"

class Bed;

class SnpFilterBase {
public:
    SnpFilterBase() : _filtered(0) {}
    ~SnpFilterBase() {}

    uint64_t filtered() {
        return _filtered;
    }

    bool exclude(const Bed& snp) {
        if (_exclude(snp)) {
            ++_filtered;
            return true;
        }
        return false;
    }

protected:
    virtual bool _exclude(const Bed& snp) = 0;

protected:
    uint64_t _filtered;
};
