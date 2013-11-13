#include "RemapCigarCommand.hpp"

#include "common/CigarString.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <sstream>

using boost::format;
using namespace std;
namespace po = boost::program_options;

RemapCigarCommand::RemapCigarCommand()
    : _inputFile("-")
    , _outputFile("-")
{
}

void RemapCigarCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<string>(&_inputFile)->default_value("-"),
            "input sam stream (default is - for stdin)")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (default is - for stdout)")
        ;

    _posOpts.add("input-file", 1);
    _posOpts.add("output-file", 1);
}

void RemapCigarCommand::exec() {
    InputStream::ptr in = _streams.openForReading(_inputFile);
    ostream* out = _streams.get<ostream>(_outputFile);

    string line;
    while (in->getline(line)) {
        string::size_type pos = line.find("ZR:Z:REMAP");
        if (pos == string::npos) {
            *out << line << "\n";
            continue;
        }

        Tokenizer<char> tokLine(line, '\t');
        vector<string> fields;
        string fld;
        while (!tokLine.eof() && tokLine.extract(fld)) {
            // we are stripping out the read vs ref diff information for now
            // as we don't want to compute the update with the remapped read.
            // samtools doesn't seem to use it for variant calling. perhaps
            // we will revisit this if something downstream turns wants it.
            if (fld.find("MD:Z:") == 0)
                continue;
            fields.push_back(fld);
        }

        string cigar = fields[5];

        string contig = &line[pos+5];
        string contigCigar;
        Tokenizer<char> tokCigar(contig, '-');
        tokCigar.advance(2);
        tokCigar.extract(contigCigar);

        uint32_t readPos(0);
        Tokenizer<char> tokPos(contig, ',');
        tokPos.advance(1);
        tokPos.extract(readPos);

        CigarString bwaCigar(cigar);
        CigarString mapCigar(contigCigar);
        mapCigar = mapCigar.structural();

        readPos -= 1; // from 1 based to 0 based
        CigarString merged(CigarString::merge(mapCigar, bwaCigar, readPos));

        ostream_iterator<string> iter(*out, "\t");
        fields[5] = string(merged);
        copy(fields.begin(), fields.end()-1, iter);
        *out << *fields.rbegin() << "\n";
    }
}
