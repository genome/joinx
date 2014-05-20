#pragma once

#include "ui/CommandBase.hpp"

#include <set>
#include <string>

class VcfRemoveFilteredGtCommand : public CommandBase {
public:
    VcfRemoveFilteredGtCommand();

    std::string name() const { return "vcf-remove-filtered-gt"; }
    std::string description() const {
        return "remove filtered sample data from vcf files";
    }

    void configureOptions();
    void exec();

protected:
    std::string inputFile_;
    std::string outputFile_;
    std::vector<std::string> whitelist_;
};
