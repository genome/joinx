#pragma once

#include "intconfig.hpp"

class Bed;
class SnpStream;
class IResultCollector;

class SnpIntersector {
public:
    SnpIntersector(SnpStream& a, SnpStream& b, IResultCollector& rc);

    void exec();

protected:
    SnpStream& _a;
    SnpStream& _b;
    IResultCollector& _rc;
};
