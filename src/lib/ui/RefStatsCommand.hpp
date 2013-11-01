#pragma once

#include "ui/CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class RefStatsCommand : public CommandBase {
public:
    RefStatsCommand();

    std::string name() const { return "ref-stats"; }
    std::string description() const {
        return "compute statistics (#A/T bases, #C/G, #CpG bases) in regions taken from a .bed file";
    }

    void configureOptions();
    void exec();

protected:
    bool _refBases;
    std::string _outFile;
    std::string _bedFile;
    std::string _fastaFile;
};


