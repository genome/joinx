#pragma once

#include "ui/CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class Vcf2RawCommand : public CommandBase {
public:
    using CommandBase::ptr;

    Vcf2RawCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "vcf2raw"; }
    std::string description() const {
        return "convert vcf files to bed files";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _vcfFile;
    std::string _outFile;
    std::string _refFa;
    StreamHandler _streams;
};

