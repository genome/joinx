#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class VcfFilterCommand : public CommandBase {
public:
    VcfFilterCommand();

    std::string name() const { return "vcf-filter"; }
    std::string description() const {
        return "filter vcf files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _infile;
    std::string _outputFile;
    uint32_t _minDepth;
};
