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

BedMergeCommand::BedMergeCommand()
    : _inputFile("-")
    , _outputFile("-")
    , _distance(0)
{
}

void BedMergeCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<string>(&_inputFile)->required(),
            "input file (empty or - for stdin)")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (empty or - means stdout, which is the default)")

        ("distance,d", po::value<size_t>(&_distance)->default_value(0),
            "Allowable distance between features to be merged (default: 0)")
        ;

    _posOpts.add("input-file", 1);
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
