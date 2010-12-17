#pragma once

#include "intconfig.hpp"

class Bed;
class BedStream;
class IResultCollector;

class SnvComparator {
public:
    SnvComparator(BedStream& a, BedStream& b, IResultCollector& rc);

    void exec();

protected:
    BedStream& _a;
    BedStream& _b;
    IResultCollector& _rc;
};
