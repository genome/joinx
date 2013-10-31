#pragma once

#include "ui/CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <string>

class VcfSiteFilterCommand : public CommandBase {
public:
    using CommandBase::ptr;

    VcfSiteFilterCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "vcf-site-filter"; }
    std::string description() const {
        return "filter vcf files using a per-site filter";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _infile;
    std::string _outputFile;
    double _minFailFilter;
    std::string _filterName;
    std::string _filterDescription;
    StreamHandler _streams;
};
