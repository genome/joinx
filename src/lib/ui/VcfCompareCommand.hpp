#pragma once

#include "ui/CommandBase.hpp"
#include "fileformats/vcf/GenotypeComparator.hpp"

#include <boost/unordered_map.hpp>

#include <string>
#include <vector>

class VcfCompareCommand : public CommandBase {
public:
    VcfCompareCommand();

    std::string name() const { return "vcf-compare"; }
    std::string description() const {
        return "compare genotypes in vcf files";
    }

    void configureOptions();
    void finalizeOptions();
    void exec();

protected:
    boost::unordered_map<std::string, std::string> sampleRenames();

protected:
    std::string outputFile_;
    std::string sampleRenameFile_;
    std::vector<std::string> sampleRenames_;
    std::vector<std::string> filenames_;
    std::vector<std::string> streamNames_;
    std::vector<Vcf::FilterType> filterTypes_;
    std::vector<std::string> filterTypeStrings_;
    std::vector<std::string> noSampleFilenames_;
    std::vector<std::string> sampleNames_;
    std::string outputDir_;
    std::string exactFormatField_;
    std::string partialFormatField_;
    bool includeRefAlleles_;
};
