#pragma once

class Bed;

#include "intconfig.hpp"

class IResultCollector {
public:
    virtual ~IResultCollector() {}

    virtual void hit(const Bed& a, const Bed& b) = 0;
    virtual void miss(const Bed& a, const Bed& b) = 0;
};
