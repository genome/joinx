#pragma once

#include "CommandBase.hpp"

#include <fstream>
#include <string>
#include <vector>

class SnvConcordanceCommand : public CommandBase {
public:
    using CommandBase::ptr;

    SnvConcordanceCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "snv-concordance"; }
    std::string description() const {
        return "produce snv concordance report for 2 variant files";
    }

    void exec();


public:
    struct Streams {
        Streams()
            : inA(NULL)
            , inB(NULL)
            , out(NULL)
            , outHit(NULL)
            , outMissA(NULL)
            , outMissB(NULL)
        {}

        ~Streams() {
            for (unsigned i = 0; i < cleanup.size(); ++i)
                delete cleanup[i];
        }

        std::istream* inA;
        std::istream* inB;
        std::ostream* out;
        std::ostream* outHit;
        std::ostream* outMissA;
        std::ostream* outMissB;

        std::vector<std::fstream*> cleanup;
    };


protected:
    void parseArguments(int argc, char** argv);
    void setupStreams(Streams& s) const;

protected:
    std::string _fileA;
    std::string _fileB;
    std::string _missFileA;
    std::string _missFileB;
    std::string _outputFile;
    std::string _hitsFile;
    bool _useDepth;
};
