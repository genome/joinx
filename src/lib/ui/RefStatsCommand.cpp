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


RefStatsCommand::RefStatsCommand()
    : _refBases(false)
    , _outFile("-")
{
}

void RefStatsCommand::configureOptions() {
    _opts.add_options()
        ("bed,b",
            po::value<string>(&_bedFile)->required(),
            "input .bed file (required, use '-' for stdin)")

        ("fasta,f",
            po::value<string>(&_fastaFile)->required(),
            "input fasta file (required, use '-' for stdin)")

        ("output,o",
            po::value<string>(&_outFile)->default_value("-"),
            "output .bed file (omit or use '-' for stdout)")

        ("ref-bases,r",
            po::bool_switch(&_refBases)->default_value(false),
            "output reference bases in each region")
    ;

    _posOpts.add("bed", 1);
    _posOpts.add("fasta", 1);
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
