#pragma once

class Bed;

class IResultCollector {
public:
    virtual ~IResultCollector() {}

    virtual void hitA(const Bed& a) = 0;
    virtual void hitB(const Bed& b) = 0;
    virtual void missA(const Bed& a) = 0;
    virtual void missB(const Bed& b) = 0;
};
