#include "VcfSiteFilterCommand.hpp"

#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>

#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;

VcfSiteFilterCommand::VcfSiteFilterCommand()
    : _infile("-")
    , _outputFile("-")
    , _minFailFilter(1.0)
{
}

void VcfSiteFilterCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<string>(&_infile)->default_value("-"),
            "input file (empty or - means stdin")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (empty or - means stdout")

        ("min-fail-filter,f",
            po::value<double>(&_minFailFilter)->default_value(1.0),
            "minimum fraction of failed samples to fail a site")
        ;

    _posOpts.add("input-file", -1);

}

void VcfSiteFilterCommand::exec() {
    //construct filter names
    stringstream name;
    name.precision(2);
    name << "sf" << _minFailFilter * 100.0;
    _filterName = name.str();

    stringstream description;
    description.precision(2);
    description << "More than " << _minFailFilter * 100.0
            << "% samples with data failed the per-sample filter";

    _filterDescription = description.str();


    InputStream::ptr instream(_streams.wrap<istream, InputStream>(_infile));
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    auto readerPtr = openVcf(*instream);
    auto& reader = *readerPtr;

    OutputWriter<Vcf::Entry> writer(*out);
    //create filter entry for header
    reader.header().addFilter(_filterName,_filterDescription);

    Vcf::Entry e;
    *out << reader.header();
    while (reader.next(e)) {
        uint32_t numSamplesEval = e.sampleData().samplesEvaluatedByFilter();
        int32_t numFailed = e.sampleData().samplesFailedFilter();
        if(numFailed < 0) {
            //there was no FT field available. Warn.
            cerr << "No per-sample filter field available for line " << reader.lineNum() << endl;
        } else {
            if (numSamplesEval && (double) numFailed/ (double) numSamplesEval > _minFailFilter) {
                e.addFilter(_filterName);
            }
            else {
                e.addFilter("PASS");
            }
            writer(e);
        }
    }
}
