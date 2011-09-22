#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <memory>

class ResultStreamWriter;

class SnvConcordanceByQualityCommand : public CommandBase {
public:
    using CommandBase::ptr;

    SnvConcordanceByQualityCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "snv-concordance-by-quality"; }
    std::string description() const {
        return "produce snv concordance report by quality for 2 variant files";
    }

    void exec();
    
    std::unique_ptr<ResultStreamWriter> setupStreamWriter();

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _fileA;
    std::string _fileB;

    std::string _hitFileA;
    std::string _hitFileB;
    std::string _missFileA;
    std::string _missFileB;

    StreamHandler _streamHandler;

    std::ofstream _hitA;
    std::ofstream _hitB;
    std::ofstream _missA;
    std::ofstream _missB;
};
