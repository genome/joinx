#include "CheckRefCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/Variant.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>

using boost::assign::list_of;
using boost::format;
using namespace std;
namespace po = boost::program_options;

CheckRefCommand::CheckRefCommand()
    : _reportFile("-")
{
}

void CheckRefCommand::configureOptions() {
    _opts.add_options()

        ("bed,b",
            po::value<string>(&_bedFile)->required(),
            "input .bed file (required, - for stdin)")

        ("fasta,f",
            po::value<string>(&_fastaFile)->required(),
            "input fasta file (required, - for stdin)")

        ("report-file,o",
            po::value<string>(&_reportFile)->default_value("-"),
            "report file (defaults to stdout)")

        ("miss-file,m",
            po::value<string>(&_missFile),
            "(optional) output entries which do not match the reference to this file")
    ;

    _posOpts.add("bed", 1);
    _posOpts.add("fasta", 1);
}

void CheckRefCommand::exec() {
    InputStream::ptr inStream = _streams.openForReading(_bedFile);
    BedReader::ptr bedReader = openBed(*inStream, 1);

    Fasta refSeq(_fastaFile);

    ostream* report = _streams.get<ostream>(_reportFile);
    ostream* miss(NULL);
    if (!_missFile.empty())
        miss = _streams.get<ostream>(_missFile);
    else
        miss = &cout;

    Bed entry;
    string referenceBases;
    uint64_t misses = 0;
    auto& reader = *bedReader;
    while (reader.next(entry)) {
        Variant v(entry);
        try {
            uint64_t len = v.stop() - v.start();
            // bed is 0-based, so we add 1 to the start position
            referenceBases = refSeq.sequence(v.chrom(), v.start()+1, len);
            if (v.reference().data() != referenceBases) {
                ++misses;
                if (miss)
                    *miss << entry << "\tREF:" << referenceBases << "\n";
            }
        } catch (const exception& e) {
            *miss << entry << "\tERROR: " << e.what() << "\n";
        }
    }

    *report << str(format("%1% %2% did not match the reference.\n")
        %misses %(misses == 1 ? "entry" : "entries"));
}
