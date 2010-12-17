#pragma once

class Bed;

#include "intconfig.hpp"

class IResultCollector {
public:
    virtual ~IResultCollector() {}

    virtual void hit(const Bed& a, const Bed& b) = 0;
    virtual void hitA(const Bed& a) = 0;
    virtual void hitB(const Bed& b) = 0;
    virtual void missA(const Bed& a) = 0;
    virtual void missB(const Bed& a) = 0;
};
