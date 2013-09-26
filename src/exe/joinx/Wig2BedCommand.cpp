#include "Wig2BedCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/WiggleReader.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <functional>

using boost::format;
using namespace std;
using namespace std::placeholders;
namespace po = boost::program_options;

CommandBase::ptr Wig2BedCommand::create(int argc, char** argv) {
    boost::shared_ptr<Wig2BedCommand> app(new Wig2BedCommand);
    app->parseArguments(argc, argv);
    return app;
}

Wig2BedCommand::Wig2BedCommand()
    : _outFile("-")
    , _stripChr(false)
    , _nonzero(false)
{
}

void Wig2BedCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("wig,w", po::value<string>(&_wigFile), "input .wig file (required, - for stdin)")
        ("output,o", po::value<string>(&_outFile), "output .bed file (- or omit for stdout)")
        ("stripchr,c", "strip leading 'chr' from sequence names (default: false)")
        ("nonzero,Z", "output nonzero entries only (default: false)")
    ;

    po::positional_options_description posOpts;
    posOpts.add("wig", 1);
    posOpts.add("output", 1);

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

    if (!vm.count("wig")) {
        stringstream ss;
        ss << "no wig file specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (vm.count("nonzero")) {
        _nonzero = true;
    }

    if (vm.count("stripchr")) {
        _stripChr = true;
    }
}

void Wig2BedCommand::exec() {
    ostream* out = _streams.get<ostream>(_outFile);
    InputStream::ptr in = _streams.wrap<istream, InputStream>(_wigFile);
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
