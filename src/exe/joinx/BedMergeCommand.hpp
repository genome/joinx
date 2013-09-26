#pragma once

#include "CommandBase.hpp"
#include "common/cstdint.hpp"
#include "fileformats/StreamHandler.hpp"

#include <string>

class BedMergeCommand : public CommandBase {
public:
    using CommandBase::ptr;

    BedMergeCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "bed-merge"; }
    std::string description() const {
        return "merge overlapping bed entries in a bed file";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _inputFile;
    std::string _outputFile;
    size_t _distance;
    StreamHandler _streams;
};
