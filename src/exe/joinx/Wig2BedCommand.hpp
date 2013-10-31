#pragma once

#include "ui/CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class Wig2BedCommand : public CommandBase {
public:
    using CommandBase::ptr;

    Wig2BedCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "wig2bed"; }
    std::string description() const {
        return "convert wiggle files to bed files";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _wigFile;
    std::string _outFile;
    bool _stripChr;
    bool _nonzero;
    StreamHandler _streams;
};

