#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class SnvConcordanceByQualityCommand : public CommandBase {
public:
    SnvConcordanceByQualityCommand();

    std::string name() const { return "snv-concordance-by-quality"; }
    std::string description() const {
        return "produce snv concordance report by quality for 2 variant files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _reportFile;
    std::string _fileA;
    std::string _fileB;

    std::string _hitFileA;
    std::string _hitFileB;
    std::string _missFileA;
    std::string _missFileB;
};
