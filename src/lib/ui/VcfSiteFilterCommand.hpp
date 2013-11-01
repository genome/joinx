#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class VcfSiteFilterCommand : public CommandBase {
public:
    VcfSiteFilterCommand();

    std::string name() const { return "vcf-site-filter"; }
    std::string description() const {
        return "filter vcf files using a per-site filter";
    }

    void configureOptions();
    void exec();

protected:
    std::string _infile;
    std::string _outputFile;
    double _minFailFilter;
    std::string _filterName;
    std::string _filterDescription;
};
