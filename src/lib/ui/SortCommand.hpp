#pragma once

#include "ui/CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class SortCommand : public CommandBase {
public:
    SortCommand();

    std::string name() const { return "sort"; }
    std::string description() const {
        return "sort variant files";
    }

    void configureOptions();
    void exec();

protected:
    std::string _outputFile;
    std::vector<std::string> _filenames;
    uint64_t _maxInMem;
    bool _mergeOnly;
    bool _stable;
    bool _unique;
    std::string _compressionString;
};
