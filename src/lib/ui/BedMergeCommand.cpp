#include "BedMergeCommand.hpp"

#include "common/cstdint.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "io/StreamJoin.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace po = boost::program_options;
using boost::format;
using namespace std;

BedMergeCommand::BedMergeCommand()
    : _inputFile("-")
    , _outputFile("-")
    , _distance(0)
    , _names(false)
    , _count(false)
    , _uniqueNames(false)
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

        ("names,n", po::bool_switch(&_names)->default_value(false),
            "Report the union of names (column 4) from merged entries")

        ("count,c", po::bool_switch(&_count)->default_value(false),
            "Report the number of merged entries in column 5")

        ("unique-names,u", po::bool_switch(&_uniqueNames)->default_value(false),
            "Only merge entries that have the same name (column 4)")

        ;

    _posOpts.add("input-file", 1);
}

bool BedMergeCommand::shouldMerge(Bed const& a, Bed const& b) const {
    auto const& exA = a.extraFields();
    auto const& exB = b.extraFields();

    bool sameName = (exA.empty() && exB.empty())
        || (!exA.empty() && !exB.empty() && exA[0] == exB[0]);

    return
        !(b.chrom() != a.chrom()
            || (b.start() > a.stop()
                && b.start() - a.stop() > int64_t(_distance)))
        && (!_uniqueNames || sameName);
}

void BedMergeCommand::exec() {
    InputStream::ptr inStream = _streams.openForReading(_inputFile);
    ostream* outStream = _streams.get<ostream>(_outputFile);

    BedReader::ptr in = openBed(*inStream, -1);
    Bed bed;
    Bed* peekBuf;
    Bed tmp;
    while (in->next(bed)) {
        size_t count(1);
        std::set<std::string> names;
        auto const& extra = bed.extraFields();

        if (_names && !extra.empty()) {
            names.insert(extra[0]);
        }

        while (in->peek(&peekBuf)) {
            if (!shouldMerge(bed, *peekBuf)) {
                break;
            }
            ++count;

            bed.stop(std::max(peekBuf->stop(), bed.stop()));
            auto const& peekExtra = peekBuf->extraFields();
            if (_names && !peekExtra.empty()) {
                names.insert(peekExtra[0]);
            }
            in->next(tmp);
        }

        if (_names && !names.empty()) {
            std::stringstream ss;
            ss << streamJoin(names).delimiter(";").emptyString(".");
            bed.setExtra(0, ss.str());
        }

        if (_count) {
            bed.setExtra(1, count);
        }

        *outStream << bed << "\n";
    }
}
