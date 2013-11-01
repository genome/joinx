#pragma once

#include "ui/CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class CheckRefCommand : public CommandBase {
public:
    CheckRefCommand();

    std::string name() const { return "check-ref"; }
    std::string description() const {
        return "compare the reference column in a bed file against a fasta";
    }

    void configureOptions();
    void exec();

protected:
    std::string _bedFile;
    std::string _fastaFile;
    std::string _missFile;
    std::string _reportFile;
};

