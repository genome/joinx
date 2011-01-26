#pragma once

#include "common/intconfig.hpp"

#include <fstream>
#include <memory>
#include <string>

class ResultStreamWriter;

class JoinXApp {
public:
    JoinXApp(int argc, char** argv);

    void usage();
    void exec();

    std::auto_ptr<ResultStreamWriter> setupStreamWriter();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _fileA;
    std::string _fileB;
    std::string _outputFile;
    bool _firstOnly;
    bool _outputBoth;
    bool _exact;
};
