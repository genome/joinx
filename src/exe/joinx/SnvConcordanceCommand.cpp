#include "SnvConcordanceCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/TypedStream.hpp"
#include "processors/IntersectFull.hpp"
#include "reports/SnvConcordance.hpp"

#include <boost/program_options.hpp>
#include <algorithm>
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
    boost::shared_ptr<SnvConcordanceCommand> app(new SnvConcordanceCommand);
    app->parseArguments(argc, argv);
    return app;
}

SnvConcordanceCommand::SnvConcordanceCommand()
    : _outputFile("-")
    , _useDepth(false)
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
        throw CmdlineHelpException(ss.str());
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

namespace {
    class Collector {
    public:
        Collector(
                SnvConcordance& concordance,
                ostream* outHit,
                ostream* outMissA,
                ostream* outMissB
                )
            : _concordance(concordance)
            , _outHit(outHit)
            , _outMissA(outMissA)
            , _outMissB(outMissB)
        {
        }

    bool hit(const Bed& a, const Bed& b) {
        if (_outHit)
            *_outHit << a << "\t" << b << endl;

        return _concordance.hit(a, b);
    }

    void missA(const Bed& a) {
        if (_outMissA)
            *_outMissA << a << "\n";;
        _concordance.missA(a);
    }

    void missB(const Bed& b) {
        if (_outMissB)
            *_outMissB << b << "\n";
        _concordance.missB(b);
    }

    // snv concordance always needs to count misses to compute percentages
    bool wantMissA() const {
        return true;
    }

    bool wantMissB() const {
        return true;
    }

    protected:
        SnvConcordance& _concordance;
        ostream* _outHit;
        ostream* _outMissA;
        ostream* _outMissB;
    };
}

void SnvConcordanceCommand::exec() {
    if (_fileA == _fileB) {
        throw runtime_error("Input files have the same name, '" + _fileA + "', not good.");
    }

    InputStream::ptr inA(_streams.wrap<istream, InputStream>(_fileA));
    InputStream::ptr inB(_streams.wrap<istream, InputStream>(_fileB));

    ostream* out = _streams.get<ostream>(_outputFile);
    ostream* outHit(0);
    ostream* outMissA(0);
    ostream* outMissB(0);

    if (!_hitsFile.empty())
        outHit = _streams.get<ostream>(_hitsFile);
    if (!_missFileA.empty())
        outMissA = _streams.get<ostream>(_missFileA);
    if (!_missFileB.empty())
        outMissB = _streams.get<ostream>(_missFileB);

    if (_streams.cinReferences() > 1)
        throw runtime_error("Multiple input streams from stdin specified. Abort.");


    typedef function<void(const BedHeader*, string&, Bed&)> Extractor;
    Extractor extractor = bind(&Bed::parseLine, _1, _2, _3, -1);
    typedef TypedStream<Bed, Extractor> BedReader;
    BedReader fa(extractor, *inA);
    BedReader fb(extractor, *inB);

    SnvConcordance::DepthOrQual depthOrQual = _useDepth ? SnvConcordance::DEPTH : SnvConcordance::QUAL;
    SnvConcordance concordance(depthOrQual);
    Collector c(concordance, outHit, outMissA, outMissB);
    IntersectFull<BedReader,BedReader,Collector> intersector(fa, fb, c);
    intersector.execute();
    concordance.reportText(*out);
}
