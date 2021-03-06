#include "RefStatsCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/TypedStream.hpp"
#include "io/InputStream.hpp"
#include "io/StreamJoin.hpp"
#include "processors/RefStats.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

RefStatsCommand::RefStatsCommand()
    : _refBases(false)
    , _outFile("-")
{
}

void RefStatsCommand::configureOptions() {
    std::vector<std::string> defTokens{
          "a/t"
        , "c/g"
        , "cg"
        };

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

        ("token,t",
            po::value<std::vector<std::string>>(&_tokens)->default_value(
                defTokens, "-c a/t -c c/g -c cg"),
            "scan for the given token. a / character represents alternation (i.e., a/t = a OR t)")

        ("ref-bases,r",
            po::bool_switch(&_refBases)->default_value(false),
            "output reference bases in each region")
    ;

    _posOpts.add("bed", 1);
    _posOpts.add("fasta", 1);
}

void RefStatsCommand::exec() {
    ostream* out = _streams.get<ostream>(_outFile);

    InputStream::ptr inStream = _streams.openForReading(_bedFile);
    auto bedReader = openStream<Bed>(inStream);
    Fasta refSeq(_fastaFile);

    RefStats refStats(_tokens, refSeq);

    Bed entry;
    *out << "#chr\tstart\tstop\t#" << streamJoin(_tokens).delimiter("\t#");

    if (_refBases)
        *out << "\tref";
    *out << "\n";

    while (bedReader->next(entry)) {
        try {
            auto result = refStats.match(entry);
            *out << entry.chrom() << "\t" << entry.start() << "\t"
                << entry.start() + result.referenceString.size();
            for (auto i = _tokens.begin(); i != _tokens.end(); ++i) {
                *out << "\t" << result.count(*i);
            }

            if (_refBases)
                *out  << "\t" << result.referenceString;
            *out << "\n";
        } catch (const exception& e) {
            cerr << entry << "\tERROR: " << e.what() << "\n";
        }
    }
}
