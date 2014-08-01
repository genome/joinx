#pragma once

#include "ui/CommandBase.hpp"

#include <boost/unordered_map.hpp>

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
    boost::unordered_map<std::string, std::string> sampleRenames();

protected:
    std::string sampleRenameFile_;
    std::vector<std::string> sampleRenames_;
    std::vector<std::string> filenames_;
    std::vector<std::string> names_;
    std::vector<std::string> noSampleFilenames_;
    std::vector<std::string> sampleNames_;
    std::string outputDir_;
};
