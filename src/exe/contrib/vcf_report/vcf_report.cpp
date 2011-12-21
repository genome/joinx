#include "fileformats/InputStream.hpp"
#include "fileformats/StreamHandler.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/VcfReader.hpp"
#include "Metrics.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <limits>

using namespace std;
using namespace std::placeholders;

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Give a filename!\n";
        return 1;
    }

    StreamHandler streamHandler;
    //try {
        InputStream::ptr instream(streamHandler.wrap<istream, InputStream>(argv[1]));
        VcfReader::ptr reader = openVcf(*instream);

        uint32_t totalSites = 0;
        Vcf::Entry entry;
        Metrics::SampleMetrics sampleMetrics;
        while (reader->next(entry)) {
            Metrics::EntryMetrics siteMetrics;
            if(entry.failedFilters().size() != 1 || entry.failedFilters().find("PASS") == entry.failedFilters().end())
                continue;
            totalSites++;
            if(!entry.hasGenotypeData())
                continue;
            siteMetrics.processEntry(entry);
            sampleMetrics.processEntry(entry,siteMetrics);
            map< Vcf::GenotypeCall, uint32_t>  distribution = siteMetrics.genotypeDistribution();
            std::vector<uint32_t> alleles = siteMetrics.allelicDistribution();
            //cout << entry.chrom() << "\t" << entry.pos();
            /*
            for( auto i = distribution.begin(); i != distribution.end(); ++i) {
                for(auto j = (*i).second.begin(); j != (*i).second.end(); ++j) {
                    cout << "\t" << (*i).first << "/" << (*j).first << "\t" << (*j).second;
                }
            }
            cout << endl;
            */
            //for(uint32_t i = 0; i < alleles.size(); ++i) {
            //    cout << "\t" << alleles[i];
            //}
            //cout << "\t" << siteMetrics.minorAlleleFrequency() << endl;
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
        cout << "SampleName\tTotalSites\tRef\tHet\tHom\tFilt\tMissing\tnonDiploid\tSingleton\tVeryRare\tRare\tCommon\tTransitions\tTransversions\tTransition:Transversion" << endl;
        for(uint32_t i = 0; i < entry.header().sampleCount(); ++i) {
            cout << entry.header().sampleNames()[i];
            cout << "\t" << totalSites;
            cout << "\t" << sampleMetrics.numRefCalls(i);
            cout << "\t" << sampleMetrics.numHetVariants(i);
            cout << "\t" << sampleMetrics.numHomVariants(i);
            cout << "\t" << sampleMetrics.numFilteredCalls(i);
            cout << "\t" << sampleMetrics.numMissingCalls(i);
            cout << "\t" << sampleMetrics.numNonDiploidCalls(i);
            cout << "\t" << sampleMetrics.numSingletonVariants(i);
            cout << "\t" << sampleMetrics.numVeryRareVariants(i);
            cout << "\t" << sampleMetrics.numRareVariants(i);
            cout << "\t" << sampleMetrics.numCommonVariants(i);
            cout << "\t" << sampleMetrics.numTransitions(i);
            cout << "\t" << sampleMetrics.numTransversions(i);
            double ratio = sampleMetrics.transitionTransversionRatio(i);
            if(ratio != std::numeric_limits<double>::quiet_NaN() && ratio < std::numeric_limits<double>::infinity()) {
                cout << "\t" << sampleMetrics.transitionTransversionRatio(i) << endl;
            }
            else {
                cout << "\tNA" << endl;
            }
        }
        //cout << "Total number of segregating sites: " << totalSites << endl;
    /*} catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }*/


    return 0;
}
