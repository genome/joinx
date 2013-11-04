#include "Wig2BedCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/WiggleReader.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <functional>

using boost::format;
using namespace std;
namespace po = boost::program_options;

Wig2BedCommand::Wig2BedCommand()
    : _outFile("-")
    , _stripChr(false)
    , _nonzero(false)
{
}

void Wig2BedCommand::configureOptions() {
    _opts.add_options()
        ("wig,w",
            po::value<string>(&_wigFile)->required(),
            "input .wig file (use '-' for stdin)")

        ("output,o",
            po::value<string>(&_outFile),
            "output .bed file (omit or use '-' for stdout)")

        ("stripchr,c",
            po::bool_switch(&_stripChr),
            "strip leading 'chr' from sequence names (default: false)")

        ("nonzero,Z",
            po::bool_switch(&_nonzero),
            "output nonzero entries only (default: false)")
    ;

    _posOpts.add("wig", 1);
    _posOpts.add("output", 1);
}

void Wig2BedCommand::exec() {
    ostream* out = _streams.get<ostream>(_outFile);
    InputStream::ptr in = _streams.openForReading(_wigFile);
    WiggleReader wr(*in, _stripChr);
    Bed bed;
    while (wr.next(bed)) {
        if (!_nonzero || (bed.extraFields().size() > 0
            && bed.extraFields()[0] != "0"))
        {
            *out << bed << "\n";
        }
    }
}
