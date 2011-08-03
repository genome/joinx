#pragma once

#include "CommandBase.hpp"
#include "StreamHandler.hpp"

#include <fstream>
#include <string>
#include <vector>

class IntersectCommand : public CommandBase {
public:
    using CommandBase::ptr;

    IntersectCommand();
    ptr create(int argc, char** argv);

    std::string name() const { return "intersect"; }
    std::string description() const {
        return "intersect variant files";
    }

    void exec();
    
protected:
    void parseArguments(int argc, char** argv);

protected:
    struct Streams {
        Streams()
            : inA(NULL)
            , inB(NULL)
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
        std::ostream* outHit;
        std::ostream* outMissA;
        std::ostream* outMissB;

        std::vector<std::fstream*> cleanup;
    };

    void setupStreams(Streams& s);

protected:
    std::string _fileA;
    std::string _fileB;
    std::string _missFileA;
    std::string _missFileB;
    std::string _outputFile;
    std::string _formatString;
    bool _firstOnly;
    bool _outputBoth;
    bool _exactPos;
    bool _exactAllele;
    bool _iubMatch;
    bool _dbsnpMatch;
    bool _adjacentInsertions;
    StreamHandler _streams;
};
