#include "RefStatsCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/InputStream.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <algorithm>
#include <cctype>
#include <functional>

using boost::format;
using namespace std;
using namespace std::placeholders;
namespace po = boost::program_options;

CommandBase::ptr RefStatsCommand::create(int argc, char** argv) {
    std::shared_ptr<RefStatsCommand> app(new RefStatsCommand);
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
        throw runtime_error(ss.str());
    }

    if (vm.count("ref-bases"))
        _refBases = true;

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

namespace {
    struct rstats {
        size_t at;
        size_t cg;
        size_t cpg;
    };

    rstats getstats(string const& ref, char prevBase, char nextBase) {
        rstats rv = { 0, 0, 0 };
        if (ref.empty())
            return rv;

        // we handle the first and last base (if set) outside of the loop to
        // check for
        // CpG
        string::size_type start(0);
        string::size_type end(ref.size());

        if (prevBase == 'C' && ref[0] == 'G') {
            ++start;
            ++rv.cpg;
        }

        if (start < end && *ref.rbegin() == 'C' && nextBase == 'G') {
            --end;
            ++rv.cpg;
        }

        for (string::size_type i = start; i < end; ++i) {
            if (ref[i] == 'A' || ref[i] == 'T') {
                ++rv.at;
            } else if (i < end-1 && ref[i] == 'C' && ref[i+1] == 'G') {
                rv.cpg += 2;
                ++i;
            } else if (ref[i] == 'C' || ref[i] == 'G') {
                ++rv.cg;
            }
        }
        return rv;
    }
}

void RefStatsCommand::exec() {
    ostream* out = _streams.get<ostream>(_outFile);

    InputStream::ptr inStream = _streams.wrap<istream, InputStream>(_bedFile);
    typedef function<void(const BedHeader*, string&, Bed&)> ExtractorType;
    ExtractorType extractor = bind(&Bed::parseLine, _1, _2, _3, 1);
    typedef TypedStream<Bed, ExtractorType> BedReader;
    BedReader bedReader(extractor, *inStream);
    Fasta refSeq(_fastaFile);

    Bed entry;
    string referenceBases;
    *out << "#chr\tstart\tstop\t#a/t\t#c/g\t#cpg";
    if (_refBases)
        *out << "\tref";
    *out << "\n";

    while (bedReader.next(entry)) {
        try {
            char prevBase(0);
            char nextBase(0);

            if (entry.start() > 0)
                prevBase = toupper(refSeq.sequence(entry.chrom(), entry.start()));

            if (size_t(entry.stop()) <= refSeq.seqlen(entry.chrom()))
                nextBase = toupper(refSeq.sequence(entry.chrom(), entry.stop()+1));

            string ref = refSeq.sequence(entry.chrom(), entry.start()+1,
                entry.stop()-entry.start());

            string ucref(ref.size(), '\0');
            transform(ref.begin(), ref.end(), ucref.begin(), ::toupper);

            rstats rs = getstats(ucref, prevBase, nextBase);
            *out << entry << "\t"
                << rs.at << "\t"
                << rs.cg << "\t"
                << rs.cpg;
            if (_refBases)
                *out  << "\t" << ref;
            *out << "\n";
        } catch (const exception& e) {
            cerr << entry << "\tERROR: " << e.what() << "\n";
        }
    }
}
