#pragma once

#include "ui/CommandBase.hpp"
#include "fileformats/vcf/MergeStrategy.hpp"

#include <map>
#include <string>

class VcfMergeCommand : public CommandBase {
public:
    VcfMergeCommand();

    std::string name() const { return "vcf-merge"; }
    std::string description() const {
        return "merge vcf files";
    }

    void configureOptions();
    void finalizeOptions();
    void exec();

protected:
    std::vector<std::string> _filenames;
    std::vector<std::string> _dupSampleFilenames;
    std::string _outputFile;
    std::string _fastaFile;
    std::string _mergeStrategyFile;
    bool _clearFilters;
    bool _mergeSamples;
    double _consensusRatio;
    std::string _consensusOpts;
    std::string _consensusFilter;
    std::string _consensusFilterDesc;
    std::map<std::string, size_t> _fileOrder;
    std::map<std::string, std::string> _dupSampleMap;

    std::string _samplePrioStr;
    Vcf::MergeStrategy::SamplePriority _samplePriority;
    bool _exactPos;
};
