#pragma once

#include "fileformats/TypedStream.hpp"

#include <cstdint>

class Bed;
class IResultCollector;

class SnvComparator {
public:
    typedef TypedStream<Bed, std::function<void(std::string&, Bed&)> > BedReader;
    SnvComparator(BedReader& a, BedReader& b, IResultCollector& rc);

    void exec();

protected:
    BedReader& _a;
    BedReader& _b;
    IResultCollector& _rc;
};
