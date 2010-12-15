#pragma once

#include "intconfig.hpp"

class Bed;
class SnvStream;
class IResultCollector;

class SnvIntersector {
public:
    SnvIntersector(SnvStream& a, SnvStream& b, IResultCollector& rc);

    void exec();

protected:
    SnvStream& _a;
    SnvStream& _b;
    IResultCollector& _rc;
};
