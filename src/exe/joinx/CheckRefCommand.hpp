#pragma once

#include "ui/CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class CheckRefCommand : public CommandBase {
public:
    using CommandBase::ptr;

    CheckRefCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "check-ref"; }
    std::string description() const {
        return "compare the reference column in a bed file against a fasta";
    }

    void exec();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _bedFile;
    std::string _fastaFile;
    std::string _missFile;
    std::string _reportFile;
    StreamHandler _streams;
};

