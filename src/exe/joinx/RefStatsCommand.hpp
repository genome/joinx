#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class RefStatsCommand : public CommandBase {
public:
    using CommandBase::ptr;

    RefStatsCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "ref-stats"; }
    std::string description() const {
        return "compare the reference column in a bed file against a fasta";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    bool _refBases;
    std::string _outFile;
    std::string _bedFile;
    std::string _fastaFile;
    StreamHandler _streams;
};


