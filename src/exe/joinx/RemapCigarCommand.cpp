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

CommandBase::ptr RemapCigarCommand::create(int argc, char** argv) {
    boost::shared_ptr<RemapCigarCommand> app(new RemapCigarCommand);
    app->parseArguments(argc, argv);
    return app;
}

RemapCigarCommand::RemapCigarCommand()
    : _inputFile("-")
    , _outputFile("-")
{
}

void RemapCigarCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value<string>(&_inputFile), "input sam stream (default is - for stdin)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (default is - for stdout)")
        ;

    po::positional_options_description posOpts;
    posOpts.add("input-file", 1);
    posOpts.add("output-file", 1);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv)
            .options(opts)
            .positional(posOpts).run(),
        vm
    );
    po::notify(vm);
}

void RemapCigarCommand::exec() {
    istream* in(NULL);
    ostream* out(NULL);

    if (_inputFile.empty() || _inputFile == "-")
        in = &cin;
    else
        in = _streams.get(_inputFile, ios::in);

    if (_outputFile.empty() || _outputFile == "-")
        out = &cout;
    else
        out = _streams.get(_outputFile, ios::out);

    string line;
    while (getline(*in, line)) {
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

        uint32_t readPos;
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
