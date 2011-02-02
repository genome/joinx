#pragma once

#include "CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class BedStream;

class SortCommand : public CommandBase {
public:
    using CommandBase::ptr;
    typedef BedStream* InputPtr;

    SortCommand();
    ~SortCommand();

    ptr create(int argc, char** argv);

    std::string name() const { return "sort"; }
    std::string description() const {
        return "sort variant files";
    }

    void exec();
    
protected:
    void parseArguments(int argc, char** argv);

protected:
    std::string _outputFile;
    std::vector<std::string> _filenames;
    std::vector<InputPtr> _inputs;
    uint64_t _maxInMem;
    bool _mergeOnly;
    bool _stable;
};
