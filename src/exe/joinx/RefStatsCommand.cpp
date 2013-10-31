#include "RefStatsCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/InputStream.hpp"
#include "processors/RefStats.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <cctype>
#include <functional>

using boost::assign::list_of;
using boost::format;
using namespace std;
namespace po = boost::program_options;

CommandBase::ptr RefStatsCommand::create(int argc, char** argv) {
    boost::shared_ptr<RefStatsCommand> app(new RefStatsCommand);
    app->parseArguments(argc, argv);
    return app;
}

RefStatsCommand::RefStatsCommand()
    : _refBases(false)
    , _outFile("-")
{
}

void RefStatsCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("bed,b", po::value<string>(&_bedFile), "input .bed file (required, - for stdin)")
        ("fasta,f", po::value<string>(&_fastaFile), "input fasta file (required, - for stdin)")
        ("output,o", po::value<string>(&_outFile), "output .bed file (default: stdout)")
        ("ref-bases,r", "output reference bases in each region (default: false)")
    ;

    po::positional_options_description posOpts;
    posOpts.add("bed", 1);
    posOpts.add("fasta", 1);

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

    if (vm.count("ref-bases")) {
        _refBases = true;
    }

    if (!vm.count("bed")) {
        stringstream ss;
        ss << "no bed file specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("fasta")) {
        stringstream ss;
        ss << "no fasta specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }
}

void RefStatsCommand::exec() {
    ostream* out = _streams.get<ostream>(_outFile);

    InputStream::ptr inStream = _streams.wrap<istream, InputStream>(_bedFile);
    auto bedReader = openBed(*inStream);
    Fasta refSeq(_fastaFile);

    Bed entry;
    *out << "#chr\tstart\tstop\t#a/t\t#c/g\t#cpg";
    if (_refBases)
        *out << "\tref";
    *out << "\n";

    std::vector<std::string> toks = list_of("CG")("A")("C")("G")("T");
    RefStats refStats(toks, refSeq);

    while (bedReader->next(entry)) {
        try {
            auto result = refStats.match(entry);
            auto at = result.count("A") + result.count("T");
            auto cg = result.count("C") + result.count("G");
            auto cpg = result.count("CG");

            *out << entry.chrom() << "\t" << entry.start() << "\t"
                << entry.start() + result.referenceString.size() << "\t"
                << at << "\t" << cg << "\t" << cpg;

            if (_refBases)
                *out  << "\t" << result.referenceString;
            *out << "\n";
        } catch (const exception& e) {
            cerr << entry << "\tERROR: " << e.what() << "\n";
        }
    }
}
