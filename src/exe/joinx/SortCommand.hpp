#pragma once

#include "CommandBase.hpp"
#include "fileformats/StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class SortCommand : public CommandBase {
public:
    using CommandBase::ptr;

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

    void sortBed();
    void sortVcf();

protected:
    std::string _outputFile;
    std::vector<std::string> _filenames;
    uint64_t _maxInMem;
    bool _mergeOnly;
    bool _stable;
    bool _unique;
    std::string _compressionString;
    StreamHandler _streamHandler;
};
