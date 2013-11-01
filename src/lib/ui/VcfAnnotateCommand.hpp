#pragma once

#include "ui/CommandBase.hpp"
#include "annotate/SimpleVcfAnnotator.hpp"
#include "common/namespaces.hpp"

#include <map>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)
class Header;
END_NAMESPACE(Vcf)

class VcfAnnotateCommand : public CommandBase {
public:
    VcfAnnotateCommand();

    std::string name() const { return "vcf-annotate"; }
    std::string description() const {
        return "annotate vcf files";
    }

    void configureOptions();
    void exec();

protected:
    void makeInfoMap();
    void postProcessArguments(Vcf::Header& header, Vcf::Header const& annoHeader);

protected:
    std::string _vcfFile;
    std::string _annoFile;
    std::string _outputFile;
    std::vector<std::string> _infoFields;
    bool _noIdents;
    bool _noInfo;

    // post-processed arguments
    std::map<std::string, InfoTranslation> _infoMap;
};
