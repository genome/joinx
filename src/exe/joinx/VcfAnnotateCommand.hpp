#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <cstdint>
#include <string>

class VcfAnnotateCommand : public CommandBase {
public:
    using CommandBase::ptr;

    VcfAnnotateCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "vcf-annotate"; }
    std::string description() const {
        return "annotate vcf files";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::vector<std::string> _filenames;
    std::string _outputFile;
    StreamHandler _streams;
};
