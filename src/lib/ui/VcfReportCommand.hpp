#pragma once

#include "ui/CommandBase.hpp"

#include <string>
#include <vector>

class VcfReportCommand : public CommandBase {
public:
    VcfReportCommand();

    std::string name() const { return "vcf-report"; }
    std::string description() const {
        return "standard reporting on vcf files";
    }

    void configureOptions();
    void exec();


protected:
    std::string _infile;
    std::string _perSampleFile;
    std::string _perSiteFile;
    std::vector<std::string> _infoFields;
};
