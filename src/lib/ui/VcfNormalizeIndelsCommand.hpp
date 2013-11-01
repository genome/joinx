#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class VcfNormalizeIndelsCommand : public CommandBase {
public:

    VcfNormalizeIndelsCommand();

    std::string name() const { return "vcf-normalize-indels"; }
    std::string description() const {
        return "merge vcf files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _inputFile;
    std::string _fastaPath;
    std::string _outputFile;
    std::string _fastaFile;
    std::string _mergeStrategyFile;
    bool _clearFilters;
    bool _mergeSamples;
};

