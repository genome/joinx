#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <cstdint>
#include <string>

class VcfMergeCommand : public CommandBase {
public:
    using CommandBase::ptr;

    VcfMergeCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "vcf-merge"; }
    std::string description() const {
        return "merge vcf files";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::vector<std::string> _filenames;
    std::string _outputFile;
    std::string _fastaFile;
    std::string _mergeStrategyFile;
    StreamHandler _streams;
    bool _clearFilters;
    bool _mergeSamples;
};
