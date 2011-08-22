#include "SnvConcordanceCommand.hpp"
#include "bedutil/SnvConcordance.hpp"

#include "bedutil/Intersect.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/TypedStream.hpp"

#include <boost/program_options.hpp>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace std::placeholders;
namespace po = boost::program_options;

CommandBase::ptr SnvConcordanceCommand::create(int argc, char** argv) {
    std::shared_ptr<SnvConcordanceCommand> app(new SnvConcordanceCommand);
    app->parseArguments(argc, argv);
    return app;
}

SnvConcordanceCommand::SnvConcordanceCommand()
    : _useDepth(false)
{
}

void SnvConcordanceCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("file-a,a", po::value<string>(&_fileA), "input .bed file a (required, - for stdin)")
        ("file-b,b", po::value<string>(&_fileB), "input .bed file b (required, - for stdin)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("depth,d", "use read depth metric in place of quality")
        ("hits", po::value<string>(&_hitsFile), "output intersection to file, discarded by default")
        ("miss-a", po::value<string>(&_missFileA), "output misses in A to file")
        ("miss-b", po::value<string>(&_missFileB), "output misses in B to file");

    po::positional_options_description posOpts;
    posOpts.add("file-a", 1);
    posOpts.add("file-b", 1);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv)
            .options(opts)
            .positional(posOpts).run(),
        vm
    );
    po::notify(vm);

    if (vm.count("depth"))
        _useDepth = true;

    if (vm.count("help")) {
        stringstream ss;
        ss << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("file-a")) {
        stringstream ss;
        ss << "no file-a specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("file-b")) {
        stringstream ss;
        ss << "no file-b specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }
}

void SnvConcordanceCommand::setupStreams(Streams& s) const {
    unsigned cinReferences = 0;
    unsigned coutReferences = 0;

    fstream* fs;

    if (_fileA != "-") {
        s.inA = fs = new fstream(_fileA.c_str(), ios::in);
        if (!*s.inA)
            throw runtime_error("Failed to open input file '" + _fileA + "'");
        s.cleanup.push_back(fs);
    } else {
        s.inA = &cin;
        ++cinReferences;
    }

    if (_fileB != "-") {
        s.inB = fs = new fstream(_fileB.c_str(), ios::in);
        if (!*s.inB)
            throw runtime_error("Failed to open input file '" + _fileB + "'");
        s.cleanup.push_back(fs);
    } else {
        s.inB = &cin;
        ++cinReferences;
    }

    if (cinReferences > 1)
        throw runtime_error("Multiple input streams from stdin specified. Abort.");

    if (!_hitsFile.empty() && _hitsFile != "-") {
        s.outHit = fs = new fstream(_hitsFile.c_str(), ios::out);
        if (!*s.outHit)
            throw runtime_error("Failed to open output file '" + _hitsFile + "'");
        s.cleanup.push_back(fs);
    } else if (_hitsFile == "-") {
        s.outHit = &cout;
        ++coutReferences; 
    }

    if (!_outputFile.empty() && _outputFile != "-") {
        s.out = fs = new fstream(_outputFile.c_str(), ios::out);
        if (!*s.out)
            throw runtime_error("Failed to open output file '" + _outputFile + "'");
        s.cleanup.push_back(fs);
    } else {
        s.out = &cout;
        ++coutReferences; 
    }

    if (!_missFileA.empty() && _missFileA != "-") {
        s.outMissA = fs = new fstream(_missFileA.c_str(), ios::out);
        if (!*s.outMissA)
            throw runtime_error("failed to open output file '" + _missFileA + "'");
        s.cleanup.push_back(fs);
    } else if (!_missFileA.empty()) {
        s.outMissA = &cout;
        ++coutReferences;
    }

    if (!_missFileB.empty() && _missFileB != "-") {
        s.outMissB = fs = new fstream(_missFileB.c_str(), ios::out);
        if (!*s.outMissB)
            throw runtime_error("failed to open output file '" + _missFileB + "'");
        s.cleanup.push_back(fs);
    } else if (!_missFileB.empty()) {
        s.outMissB = &cout;
        ++coutReferences;
    }

    if (coutReferences > 1)
        throw runtime_error("Multiple output streams to stdout specified. Abort.");
}

namespace {
    class Collector {
    public:
        Collector(SnvConcordance& concordance, SnvConcordanceCommand::Streams& s)
            : _concordance(concordance)
            , _s(s)
        {
        }

    bool hit(const Bed& a, const Bed& b) {
        if (_s.outHit)
            *_s.outHit << a << "\t" << b << endl;
 
        return _concordance.hit(a, b);
    }

    void missA(const Bed& a) {
        if (_s.outMissA)
            *_s.outMissA << a << "\n";;
        _concordance.missA(a);
    }

    void missB(const Bed& b) {
        if (_s.outMissB)
            *_s.outMissB << b << "\n";
        _concordance.missB(b);
    }

    protected:
        SnvConcordance& _concordance;
        SnvConcordanceCommand::Streams& _s;
    };
}

void SnvConcordanceCommand::exec() {
    if (_fileA == _fileB) {
        throw runtime_error("Input files have the same name, '" + _fileA + "', not good.");
    }

    Streams s;
    setupStreams(s);

    InputStream inA(_fileA, *s.inA);
    InputStream inB(_fileB, *s.inB);
    function<void(string&, Bed&)> extractor = bind(&Bed::parseLine, _1, _2, -1);
    typedef TypedStream<Bed, function<void(string&, Bed&)> > BedReader;
    BedReader fa(extractor, inA);
    BedReader fb(extractor, inB);

    SnvConcordance::DepthOrQual depthOrQual = _useDepth ? SnvConcordance::DEPTH : SnvConcordance::QUAL;
    SnvConcordance concordance(depthOrQual);
    Collector c(concordance, s);
    Intersect<BedReader,BedReader,Collector> intersector(fa, fb, c);
    intersector.execute();
    concordance.reportText(*s.out);
}
