#include "CheckRefCommand.hpp"

#include "fileformats/Variant.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/BedStream.hpp"
#include "fileformats/FastaReader.hpp"
#include "fileformats/InputStream.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using boost::format;
using namespace std;
namespace po = boost::program_options;

CommandBase::ptr CheckRefCommand::create(int argc, char** argv) {
    std::shared_ptr<CheckRefCommand> app(new CheckRefCommand);
    app->parseArguments(argc, argv);
    return app;
}

CheckRefCommand::CheckRefCommand()
    : _reportFile("-")
{
}

void CheckRefCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("bed,b", po::value<string>(&_bedFile), "input .bed file (required, - for stdin)")
        ("fasta,f", po::value<string>(&_fastaFile), "input fasta file (required, - for stdin)")
        ("report-file,o", po::value<string>(&_reportFile), "report file (empty or - means stdout, which is the default)")
        ("miss-file,m", po::value<string>(&_missFile), "output entries which do not match the reference to this file")
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

void CheckRefCommand::exec() {
    InputStream::ptr inStream = _streams.wrap<istream, InputStream>(_bedFile);
    BedStream bedStream(*inStream, 1);
    FastaReader refSeq(_fastaFile);

    ostream* report = _streams.get<ostream>(_reportFile);
    ostream* miss(NULL);
    if (!_missFile.empty())
        miss = _streams.get<ostream>(_missFile);

    Bed entry;
    string referenceBases;
    uint64_t misses = 0;
    while (bedStream.next(entry)) {
        Variant v(entry);
        // bed is 0-based, so we add 1 to the start position
        refSeq.sequence(v.chrom(), v.start(), v.stop(), referenceBases);
        if (v.reference().data() != referenceBases) {
            ++misses;
            if (miss)
                *miss << entry << "\tREF:" << referenceBases << "\n";
        }
    }

    *report << str(format("%1% %2% did not match the reference.\n")
        %misses %(misses == 1 ? "entry" : "entries"));
}
