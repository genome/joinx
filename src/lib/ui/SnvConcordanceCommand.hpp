#pragma once

#include "ui/CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class SnvConcordanceCommand : public CommandBase {
public:
    SnvConcordanceCommand();

    std::string name() const { return "snv-concordance"; }
    std::string description() const {
        return "produce snv concordance report for 2 variant files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _fileA;
    std::string _fileB;
    std::string _missFileA;
    std::string _missFileB;
    std::string _outputFile;
    std::string _hitsFile;
    bool _useDepth;
};
