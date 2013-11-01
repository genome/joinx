#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class GenerateCommand : public CommandBase {
public:
    GenerateCommand();

    std::string name() const { return "generate"; }
    std::string description() const {
        return "generate random bed or vcf files";
    }

    void configureOptions();
    void exec();

    // hide from usage
    bool hidden() const { return true; }

protected:
    uint32_t _lines;
    uint32_t _seed;
    std::string _format;
    std::string _outputFile;
};
