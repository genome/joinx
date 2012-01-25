#pragma once

#include "CommandBase.hpp"
#include "annotate/SimpleVcfAnnotator.hpp"
#include "common/namespaces.hpp"
#include "fileformats/StreamHandler.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)
class Header;
END_NAMESPACE(Vcf)

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
    void makeInfoMap();
    void postProcessArguments(Vcf::Header& header, Vcf::Header const& annoHeader);

protected:
    std::string _vcfFile;
    std::string _annoFile;
    std::string _outputFile;
    std::vector<std::string> _infoFields;
    bool _copyIdents;
    bool _copyInfo;

    // post-processed arguments
    std::map<std::string, InfoTranslation> _infoMap;

    StreamHandler _streams;
};
