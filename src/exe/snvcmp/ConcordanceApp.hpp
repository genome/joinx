#pragma once

#include "bedutil/intconfig.hpp"

#include <fstream>
#include <memory>
#include <string>

class ResultStreamWriter;

class ConcordanceApp {
public:
    ConcordanceApp(int argc, char** argv);

    void usage();
    void exec();

    std::auto_ptr<ResultStreamWriter> setupStreamWriter();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _fileA;
    std::string _fileB;

    std::string _hitFileA;
    std::string _hitFileB;
    std::string _missFileA;
    std::string _missFileB;

    std::ofstream _hitA;
    std::ofstream _hitB;
    std::ofstream _missA;
    std::ofstream _missB;
};
