#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class SnvConcordanceCommand : public CommandBase {
public:
    using CommandBase::ptr;

    SnvConcordanceCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "snv-concordance"; }
    std::string description() const {
        return "produce snv concordance report for 2 variant files";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _fileA;
    std::string _fileB;
    std::string _missFileA;
    std::string _missFileB;
    std::string _outputFile;
    std::string _hitsFile;
    bool _useDepth;
    StreamHandler _streams;
};
