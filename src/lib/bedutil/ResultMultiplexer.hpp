#pragma once

class Bed;

#include "IResultCollector.hpp"

#include <boost/bind.hpp>
#include <algorithm>
#include <vector>

class ResultMultiplexer : public IResultCollector {
public:
    void add(IResultCollector* rc) {
        _c.push_back(rc);
    }

    void hitA(const Bed& a) {
        std::for_each(_c.begin(), _c.end(), boost::bind(&IResultCollector::hitA, _1, a));
    }
    void hitB(const Bed& b) {
        std::for_each(_c.begin(), _c.end(), boost::bind(&IResultCollector::hitB, _1, b));
    }
    void missA(const Bed& a) {
        std::for_each(_c.begin(), _c.end(), boost::bind(&IResultCollector::missA, _1, a));
    }
    void missB(const Bed& b) {
        std::for_each(_c.begin(), _c.end(), boost::bind(&IResultCollector::missB, _1, b));
    }

protected:
    std::vector<IResultCollector*> _c;
};
