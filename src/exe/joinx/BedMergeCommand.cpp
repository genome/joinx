#include "BedMergeCommand.hpp"

#include "common/cstdint.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/TypedStream.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <deque>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

namespace po = boost::program_options;
using boost::format;
using namespace std;

CommandBase::ptr BedMergeCommand::create(int argc, char** argv) {
    boost::shared_ptr<BedMergeCommand> app(new BedMergeCommand);
    app->parseArguments(argc, argv);
    return app;
}

BedMergeCommand::BedMergeCommand()
    : _inputFile("-")
    , _outputFile("-")
    , _distance(0)
{
}

void BedMergeCommand::parseArguments(int argc, char** argv) {
    string consensusOpts;
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value<string>(&_inputFile), "input file (empty or - for stdin)")
        ("output-file,o", po::value<string>(&_outputFile),
            "output file (empty or - means stdout, which is the default)")
        ("distance,d", po::value<size_t>(&_distance),
            "Allowable distance between features to be merged (default: 0)")
        ;
    po::positional_options_description posOpts;
    posOpts.add("input-file", -1);

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
        throw CmdlineHelpException(ss.str());
    }
}

void BedMergeCommand::exec() {
    InputStream::ptr inStream = _streams.wrap<istream, InputStream>(_inputFile);
    ostream* outStream = _streams.get<ostream>(_outputFile);

    BedReader::ptr in = openBed(*inStream, 0);
    Bed bed;
    Bed* peekBuf;
    Bed tmp;
    while (in->next(bed)) {
        while (in->peek(&peekBuf)) {
            if (peekBuf->chrom() != bed.chrom()
                || (peekBuf->start() > bed.stop()
                    && peekBuf->start() - bed.stop() > int64_t(_distance)))
            {
                break;
            }
            bed.stop(peekBuf->stop());
            in->next(tmp);
        }
        *outStream << bed << "\n";
    }
}
