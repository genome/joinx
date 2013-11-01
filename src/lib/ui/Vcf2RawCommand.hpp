#pragma once

#include "ui/CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class Vcf2RawCommand : public CommandBase {
public:
    Vcf2RawCommand();

    std::string name() const { return "vcf2raw"; }
    std::string description() const {
        return "convert vcf files to bed files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _vcfFile;
    std::string _outFile;
    std::string _refFa;
};

