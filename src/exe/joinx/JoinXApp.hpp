#pragma once

#include "common/intconfig.hpp"

#include <fstream>
#include <string>
#include <vector>

class ResultStreamWriter;

class JoinXApp {
public:
    JoinXApp(int argc, char** argv);

    void usage();
    void exec();


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

    void parseArguments(int argc, char** argv);
    void setupStreams(Streams& s) const;

protected:
    std::string _fileA;
    std::string _fileB;
    std::string _missFileA;
    std::string _missFileB;
    std::string _outputFile;
    bool _firstOnly;
    bool _outputBoth;
    bool _exactPos;
    bool _exactAllele;
};
