#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class CreateContigsCommand : public CommandBase {
public:
    CreateContigsCommand();

    std::string name() const { return "create-contigs"; }
    std::string description() const {
        return "generate contigs from variant files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _referenceFasta;
    std::string _variantsFile;
    std::string _outputFasta;
    std::string _outputRemap;
    int _flankSize;
    int _minQuality;
};
