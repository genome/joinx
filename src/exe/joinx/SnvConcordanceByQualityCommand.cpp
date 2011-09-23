#include "SnvConcordanceByQualityCommand.hpp"

#include "bedutil/ConcordanceQuality.hpp"
#include "bedutil/NoReferenceFilter.hpp"
#include "bedutil/ResultMultiplexer.hpp"
#include "bedutil/ResultStreamWriter.hpp"
#include "bedutil/SnvComparator.hpp"
#include "bedutil/TypeFilter.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/InputStream.hpp"

#include <boost/program_options.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace std::placeholders;
namespace po = boost::program_options;

SnvConcordanceByQualityCommand::SnvConcordanceByQualityCommand() {
}

CommandBase::ptr SnvConcordanceByQualityCommand::create(int argc, char** argv) {
    std::shared_ptr<SnvConcordanceByQualityCommand> app(new SnvConcordanceByQualityCommand);
    app->parseArguments(argc, argv);
    return app;
}

void SnvConcordanceByQualityCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("file-a,a", po::value<string>(&_fileA), "input file a (required)")
        ("file-b,b", po::value<string>(&_fileB), "input file b (required)")
        ("hits-a",   po::value<string>(&_hitFileA), "output hits in 'a' to this file")
        ("hits-b",   po::value<string>(&_hitFileB), "output hits in 'b' to this file")
        ("miss-a",   po::value<string>(&_missFileA), "output misses in 'a' to this file")
        ("miss-b",   po::value<string>(&_missFileB), "output misses in 'b' to this file");

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

unique_ptr<ResultStreamWriter> SnvConcordanceByQualityCommand::setupStreamWriter() {
    unique_ptr<ResultStreamWriter> resultStreamWriter;

    ofstream* hitA(NULL);
    ofstream* hitB(NULL);
    ofstream* missA(NULL);
    ofstream* missB(NULL);

    if (!_hitFileA.empty()) {
        _hitA.open(_hitFileA.c_str(), ios::out|ios::binary);
        if (!_hitA)
            throw runtime_error("Failed to open output file for hits in 'a': '" + _hitFileA + "'");
        hitA = &_hitA;
    }

    if (!_hitFileB.empty()) {
        _hitB.open(_hitFileB.c_str(), ios::out|ios::binary);
        if (!_hitB)
            throw runtime_error("Failed to open output file for hits in 'a': '" + _hitFileB + "'");
        hitB = &_hitB;
    }

    if (!_missFileA.empty()) {
        _missA.open(_missFileA.c_str(), ios::out|ios::binary);
        if (!_missA)
            throw runtime_error("Failed to open output file for misss in 'a': '" + _missFileA + "'");
        missA = &_missA;
    }

    if (!_missFileB.empty()) {
        _missB.open(_missFileB.c_str(), ios::out|ios::binary);
        if (!_missB)
            throw runtime_error("Failed to open output file for misss in 'a': '" + _missFileB + "'");
        missB = &_missB;
    }

    resultStreamWriter.reset(new ResultStreamWriter(hitA, hitB, missA, missB));
    return resultStreamWriter;
}

void SnvConcordanceByQualityCommand::exec() {
    typedef TypedStream<Bed, function<void(string&, Bed&)> > BedReader;

    InputStream::ptr inStreamA(_streamHandler.wrap<istream, InputStream>(_fileA));
    InputStream::ptr inStreamB(_streamHandler.wrap<istream, InputStream>(_fileB));

    // set up input filters, keep SNV only, and reject entries with N ref value
    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);

    function<void(string&, Bed&)> extractorA = bind(&Bed::parseLine, _1, _2, 2);
    BedReader fa(extractorA, *inStreamA);
    fa.addFilter(&snvOnly);
    fa.addFilter(&nref);

    function<void(string&, Bed&)> extractorB = bind(&Bed::parseLine, _1, _2, 0);
    BedReader fb(extractorB, *inStreamB);
    fb.addFilter(&snvOnly);

    ConcordanceQuality qc;
    ResultMultiplexer rmux;
    rmux.add(&qc);

    unique_ptr<ResultStreamWriter> resultStreamWriter = setupStreamWriter();
    if (resultStreamWriter.get())
        rmux.add(resultStreamWriter.get());
    SnvComparator snvi(fa, fb, rmux);
    snvi.exec();
    qc.report(cout);

    cout << "Total Snvs: " << fa.valueCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;
}
