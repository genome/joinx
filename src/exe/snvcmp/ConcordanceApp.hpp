#pragma once

#include "bedutil/intconfig.hpp"

#include <string>

class ConcordanceApp {
public:
    ConcordanceApp(int argc, char** argv);

    void usage();
    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _fileA;
    std::string _fileB;
};
