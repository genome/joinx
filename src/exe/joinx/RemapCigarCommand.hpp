#pragma once

#include "ui/CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <string>

class RemapCigarCommand : public CommandBase {
public:
    using CommandBase::ptr;

    RemapCigarCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "remap-cigar"; }
    std::string description() const {
        return "update cigar strings from remapped sam files";
    }

    void exec();

    // our little secret... (this cmd won't show in the help)
    bool hidden() const {
        return true;
    }

protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _inputFile;
    std::string _outputFile;
    StreamHandler _streams;
};
