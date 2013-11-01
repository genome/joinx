#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class IntersectCommand : public CommandBase {
public:
    IntersectCommand();

    std::string name() const { return "intersect"; }
    std::string description() const {
        return "intersect variant files";
    }

    void configureOptions();
    void finalizeOptions();
    void exec();

protected:
    std::string _fileA;
    std::string _fileB;
    std::string _missFileA;
    std::string _missFileB;
    std::string _outputFile;
    std::string _formatString;
    bool _firstOnly;
    bool _outputBoth;
    bool _exactPos;
    bool _exactAllele;
    bool _iubMatch;
    bool _dbsnpMatch;
    bool _adjacentInsertions;
};
