#pragma once

class Bed;

#include "IResultCollector.hpp"

#include <vector>

class ResultMultiplexer : public IResultCollector {
public:
    void add(IResultCollector* rc) {
        _c.push_back(rc);
    }

    void hitA(const Bed& a) {
        for (IterType iter = _c.begin(); iter != _c.end(); ++iter)
            (*iter)->hitA(a);
    }
    void hitB(const Bed& b) {
        for (IterType iter = _c.begin(); iter != _c.end(); ++iter)
            (*iter)->hitB(b);
    }
    void missA(const Bed& a) {
        for (IterType iter = _c.begin(); iter != _c.end(); ++iter)
            (*iter)->missA(a);
    }
    void missB(const Bed& b) {
        for (IterType iter = _c.begin(); iter != _c.end(); ++iter)
            (*iter)->missB(b);
    }

protected:
    typedef std::vector<IResultCollector*>::iterator IterType;
    std::vector<IResultCollector*> _c;
};
