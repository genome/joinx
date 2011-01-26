#pragma once

#include "common/intconfig.hpp"

#include <string>

class IGenomicPosition {
public:
    virtual ~IGenomicPosition() {}

    virtual const std::string& chrom() const = 0;
    virtual int64_t start() const = 0;
    virtual int64_t stop() const = 0;
};
