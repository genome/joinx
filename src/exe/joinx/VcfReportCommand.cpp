#include "VcfReportCommand.hpp"

#include "common/MutationSpectrum.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/StreamHandler.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/VcfReader.hpp"
#include "metrics/Metrics.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <limits>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using namespace std::placeholders;

CommandBase::ptr VcfReportCommand::create(int argc, char** argv) {
    std::shared_ptr<VcfReportCommand> app(new VcfReportCommand);
    app->parseArguments(argc, argv);
    return app;
}

VcfReportCommand::VcfReportCommand()
    : _infile("-")
      , _perSampleFile("per_sample_report.txt")
      , _perSiteFile("per_site_report.txt")
{
}

void VcfReportCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value<string>(&_infile), "input file (empty or - means stdin, which is the default)")
        ("per-sample-file,S", po::value<string>(&_perSampleFile), "per sample report output file")
        ("per-site-file,s", po::value<string>(&_perSiteFile), "per site report output file")
        ;
    po::positional_options_description posOpts;
    posOpts.add("input-file", -1);

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
}

void VcfReportCommand::exec() {
    InputStream::ptr instream(_streams.wrap<istream, InputStream>(_infile));
    ostream* perSampleOut = _streams.get<ostream>(_perSampleFile);
    ostream* perSiteOut = _streams.get<ostream>(_perSiteFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");
    uint32_t totalSites = 0;
    VcfReader::ptr reader = openVcf(*instream);
    Vcf::Entry entry;
    Metrics::SampleMetrics sampleMetrics(reader->header().sampleCount());
    while (reader->next(entry)) {
        Metrics::EntryMetrics siteMetrics;
        if(entry.failedFilters().size() != 1 || entry.failedFilters().find("PASS") == entry.failedFilters().end())
            continue;
        totalSites++;
        if(!entry.sampleData().hasGenotypeData())
            continue;
        siteMetrics.processEntry(entry);
        sampleMetrics.processEntry(entry,siteMetrics);

        //plotting the above distribution in R
        //ggplot(x, aes(x=V7,y = ..count../sum(..count..))) + geom_histogram() + xlab("Minor Allele Frequency") + ylab("Frequency") + opts(title = "Minor Allele Frequency Distribution")
        //ggplot(x, aes(x=V7)) + geom_histogram() + xlab("Minor Allele Frequency") + ylab("Count") + opts(title = "Minor Allele Frequency Distribution")

        //next want to add to per-sample variant metrics
        //use MAF and distribution to classify
        //if singleton (ie this is the only sample to have that particular alt. index here then go in singleton bini
        //if not singleton but MAF < 1% then rare
        //if not singleton or rarest but MAF < 5% then less rare
        //if MAF >= 5% then common
        //for each sample report total number of no data, filtered, pass_filter, singleton, rarest, rare, common SNPs, in dbSNP, Singleton Transitions, Singleton Transversions, Non-singleton Transitions, Non-singleton Transversions

        // how many samples have a non-reference genotype?
        //cout << samplesWithNonRefGenotypes(entry) << "\n";
    }
    *perSampleOut << "SampleName\tTotalSites\tRef\tHet\tHom\tFilt\tMissing\tnonDiploid\tSingleton\tVeryRare\tRare\tCommon\tTransitions\tTransversions\tTransition:Transversion" << endl;
    for(uint32_t i = 0; i < entry.header().sampleCount(); ++i) {
        *perSampleOut << entry.header().sampleNames()[i];
        *perSampleOut << "\t" << totalSites;
        *perSampleOut << "\t" << sampleMetrics.numRefCalls(i);
        *perSampleOut << "\t" << sampleMetrics.numHetVariants(i);
        *perSampleOut << "\t" << sampleMetrics.numHomVariants(i);
        *perSampleOut << "\t" << sampleMetrics.numFilteredCalls(i);
        *perSampleOut << "\t" << sampleMetrics.numMissingCalls(i);
        *perSampleOut << "\t" << sampleMetrics.numNonDiploidCalls(i);
        *perSampleOut << "\t" << sampleMetrics.numSingletonVariants(i);
        *perSampleOut << "\t" << sampleMetrics.numVeryRareVariants(i);
        *perSampleOut << "\t" << sampleMetrics.numRareVariants(i);
        *perSampleOut << "\t" << sampleMetrics.numCommonVariants(i);
        MutationSpectrum const& spectrum = sampleMetrics.mutationSpectrum(i);
        *perSampleOut << "\t" << spectrum.transitions();
        *perSampleOut << "\t" << spectrum.transversions();
        double ratio = spectrum.transitionTransversionRatio();
        if(ratio != std::numeric_limits<double>::infinity()) {
            cout << "\t" << ratio;
        }
        else {
            cout << "\tNA";
        }
        cout << "\n";
    }
    //cout << "Total number of segregating sites: " << totalSites << endl;
}
