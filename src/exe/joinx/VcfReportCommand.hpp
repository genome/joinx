#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <string>
#include <vector>

class VcfReportCommand : public CommandBase {
public:
    using CommandBase::ptr;

    VcfReportCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "vcf-report"; }
    std::string description() const {
        return "standard reporting on vcf files";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _infile;
    std::string _perSampleFile;
    std::string _perSiteFile;
    std::vector<std::string> _infoFields;
    StreamHandler _streams;
};
