#pragma once

#include "CommandBase.hpp"
#include "StreamHandler.hpp"

#include <string>

class GenerateCommand : public CommandBase {
public:
    using CommandBase::ptr;

    GenerateCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "generate"; }
    std::string description() const {
        return "generate random bed or vcf files";
    }

    void exec();
    // hide from usage
    bool hidden() const { return true; }
    
protected:
    void parseArguments(int argc, char** argv);

protected:
    uint32_t _lines;
    uint32_t _seed;
    std::string _format;
    std::string _outputFile;
    StreamHandler _streams;
};
