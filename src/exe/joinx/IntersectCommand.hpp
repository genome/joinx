#pragma once

#include "CommandBase.hpp"
#include "StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class IntersectCommand : public CommandBase {
public:
    using CommandBase::ptr;

    IntersectCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "intersect"; }
    std::string description() const {
        return "intersect variant files";
    }

    void exec();
    
protected:
    void parseArguments(int argc, char** argv);

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
    StreamHandler _streams;
};
