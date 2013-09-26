#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <string>

class VcfNormalizeIndelsCommand : public CommandBase {
public:
    using CommandBase::ptr;

    VcfNormalizeIndelsCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "vcf-normalize-indels"; }
    std::string description() const {
        return "merge vcf files";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _inputFile;
    std::string _fastaPath;
    std::string _outputFile;
    std::string _fastaFile;
    std::string _mergeStrategyFile;
    StreamHandler _streams;
    bool _clearFilters;
    bool _mergeSamples;
};

