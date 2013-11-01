#pragma once

#include "ui/CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class Wig2BedCommand : public CommandBase {
public:
    Wig2BedCommand();

    std::string name() const { return "wig2bed"; }
    std::string description() const {
        return "convert wiggle files to bed files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _wigFile;
    std::string _outFile;
    bool _stripChr;
    bool _nonzero;
};
