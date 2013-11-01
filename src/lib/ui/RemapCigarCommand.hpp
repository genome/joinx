#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class RemapCigarCommand : public CommandBase {
public:
    RemapCigarCommand();

    std::string name() const { return "remap-cigar"; }
    std::string description() const {
        return "update cigar strings from remapped sam files";
    }

    void configureOptions();
    void exec();

    // our little secret... (this cmd won't show in the help)
    bool hidden() const { return true; }

protected:
    std::string _inputFile;
    std::string _outputFile;
};
