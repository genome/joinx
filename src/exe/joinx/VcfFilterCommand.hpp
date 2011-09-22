#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <string>

class VcfFilterCommand : public CommandBase {
public:
    using CommandBase::ptr;

    VcfFilterCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "vcf-filter"; }
    std::string description() const {
        return "filter vcf files";
    }

    void exec();
    
protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _infile;
    std::string _outputFile;
    uint32_t _minDepth;
    StreamHandler _streams;
};
