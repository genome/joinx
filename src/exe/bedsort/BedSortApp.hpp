#pragma once

#include "common/intconfig.hpp"

#include <string>

class BedSortApp {
public:
    BedSortApp(int argc, char** argv);

    void usage();
    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _fileIn;
    std::string _fileOut;
    uint32_t _maxLinesInMem;
};
