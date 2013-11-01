#include "GenerateCommand.hpp"

#include "fileformats/Bed.hpp"

#include <cstdlib>
#include <ctime>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using boost::format;
using boost::lexical_cast;
using namespace std;

GenerateCommand::GenerateCommand()
    : _lines(100)
    , _seed(time(NULL))
    , _format("bed")
    , _outputFile("-")
{
}

void GenerateCommand::configureOptions() {
    _opts.add_options()
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("lines,n", po::value<uint32_t>(&_lines)->default_value(100), "# of lines to generate")
        ("format,f", po::value<string>(&_format), "vcf or bed (default: bed)")
        ("seed,s", po::value<uint32_t>(&_seed), "rng seed (defaults to current time)")
        ;

    _posOpts.add("output-file", 1);
    _posOpts.add("lines", 1);
    _posOpts.add("format", 1);
}

namespace {
    double u() {
        return rand()/(double(RAND_MAX)+1);
    }

    Bed generateBed(double pIndel, uint32_t maxIndelLength) {
        const char *alleles = "ACTG";

        uint32_t chrom = uint32_t(u()*21+1);
        bool indel = u() < pIndel;
        string ref;
        string var;
        int64_t start = u() * 100000000;
        int64_t stop;
        if (indel) {
            uint32_t len = u()*maxIndelLength+1;
            stop = start+len;
            for (uint32_t i = 0; i < len; ++i)
                ref += alleles[int(u()*4)];
            var = "*";
        } else {
            stop = start+1;
            ref = alleles[int(u()*4)];
            while ((var = alleles[int(u()*4)]) == ref);
        }

        uint32_t qual = uint32_t(u() * 80);
        uint32_t depth = u() * 100+1;

        vector<string> extra;
        extra.push_back(str(format("%1%/%2%") %ref %var));
        extra.push_back(lexical_cast<string>(qual));
        extra.push_back(lexical_cast<string>(depth));
        return Bed(lexical_cast<string>(chrom), start, stop, extra);
    }
}


void GenerateCommand::exec() {
    srand(_seed);
    if (_format != "bed") {
        cerr << "No support for format '" << _format << "'\n";
    }

    ostream* out = _streams.get<ostream>(_outputFile);
    for (uint32_t i = 0; i < _lines; ++i)
        *out << generateBed(0.08, 20) << "\n";
}
