#pragma once

#include "ui/CommandBase.hpp"

#include <string>
#include <vector>

class VcfCompareGtCommand : public CommandBase {
public:
    VcfCompareGtCommand();

    std::string name() const { return "vcf-compare-gt"; }
    std::string description() const {
        return "compare genotypes in vcf files";
    }

    void configureOptions();
    void exec();

protected:
    std::vector<std::string> filenames_;
    std::vector<std::string> noSampleFilenames_;
    std::vector<std::string> sampleNames_;
};
