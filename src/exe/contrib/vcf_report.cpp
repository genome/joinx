#include "fileformats/InputStream.hpp"
#include "fileformats/StreamHandler.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/VcfReader.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>

using namespace std;
using namespace std::placeholders;

namespace {

bool customTypeIdMatches(string const& id, Vcf::CustomType const* type) {
    return type && type->id() == id;
}

uint32_t samplesWithNonRefGenotypes(const Vcf::Entry& entry) {
    uint32_t rv(0);
    if (!entry.hasGenotypeData())
        return 0;

    for (uint32_t i = 0; i < entry.header().sampleCount(); ++i) {
        Vcf::GenotypeCall gt = entry.genotypeForSample(i);
        if (gt.empty())
            continue;

        // find the first thing that is not_equal_to 0, i.e., non-ref
        auto iter = find_if(gt.begin(), gt.end(), bind(not_equal_to<uint32_t>(), 0, _1));
        if (iter != gt.end()) // got one? increment!
            ++rv;
    }

    return rv;
}

typedef std::map<Vcf::GenotypeCall,uint32_t> genotypes;
genotypes genotypeDistribution(const Vcf::Entry& entry) {
    genotypes distribution;  //missing genotypes are not counted, but perhaps should be counted separately

    auto it = find_if(entry.formatDescription().begin(), entry.formatDescription().end(), bind(&customTypeIdMatches, "FT", _1));

    int32_t offset;
    if (it != entry.formatDescription().end()) {
        offset = distance(entry.formatDescription().begin(), it);
    }
    else {
        offset = -1;
    }
         

    for (uint32_t i = 0; i < entry.header().sampleCount(); ++i) {

        if (offset >= 0) {
            const std::string *filter;
            if (entry.sampleData(i,"FT") != NULL && (filter = entry.sampleData(i,"FT")->get<std::string>(0)) != NULL &&  *filter != "PASS")
                continue;
        }
        //if no FT then we assume all have passed :-(
        Vcf::GenotypeCall gt = entry.genotypeForSample(i);
        if(gt.size() == 0) {
            continue;
        }
        if (gt.size() != 2) {
            //anything but diploid is not supported until we understand a bit better how to represent them
            cerr << "Non-diploid genotype for sample " << entry.header().sampleNames()[i] << " skipped at position " << entry.chrom() << "\t" << entry.pos() << endl;
            continue;
        }
        else {
            distribution[gt]++; //probably ok since should use default contructor of new element before adding 1;
        }
    }

    return distribution;
}
//this returns the allele distribution for an entry
std::vector<uint32_t> alleleDistribution(const genotypes& genotypeDistribution) {
    std::vector<uint32_t> distribution(4,0);  //missing genotypes are not counted, but perhaps should be counted separately. TODO This should only really work for SNPs
    for( auto i = genotypeDistribution.begin(); i != genotypeDistribution.end(); ++i) {
        for(auto j = (*i).first.begin(); j != (*i).first.end(); ++j) {
            distribution[(*j)] += (*i).second;
        }
    }
    return distribution;
}

bool minorAlleleSort (int i,int j) { return (i != 0 && i<j); }

double minorAlleleFrequency( std::vector<uint32_t>& distribution ) {
    uint32_t totalAlleles = accumulate(distribution.begin(), distribution.end(), 0);
    return (double) *min_element(distribution.begin(),distribution.end(),minorAlleleSort) / totalAlleles; 
}
/* For Ts:Tv
 * 1) If ref base is not an A or a C then reverse complement all bases
 * 2) Then refbase -> variant are transitions
 *          A -> G - Transition
 *          C -> T - Transition
 * 3) Will also need to know the number of samples with the allele if want to segregate into singleton/non-singleton         
 */
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Give a filename!\n";
        return 1;
    }

    StreamHandler streamHandler;
    try {
        InputStream::ptr instream(streamHandler.wrap<istream, InputStream>(argv[1]));
        VcfReader::ptr reader = openVcf(*instream);

        uint32_t totalSites = 0;
        Vcf::Entry entry;
        while (reader->next(entry)) {
            if(entry.failedFilters().size() != 1 || entry.failedFilters().find("PASS") == entry.failedFilters().end())
                continue;
            totalSites++;
            if(!entry.hasGenotypeData())
                continue;
            genotypes distribution = genotypeDistribution(entry);
            std::vector<uint32_t> alleles = alleleDistribution(distribution);
            cout << entry.chrom() << "\t" << entry.pos();
            /*
            for( auto i = distribution.begin(); i != distribution.end(); ++i) {
                for(auto j = (*i).second.begin(); j != (*i).second.end(); ++j) {
                    cout << "\t" << (*i).first << "/" << (*j).first << "\t" << (*j).second;
                }
            }
            cout << endl;
            */
            for(uint32_t i = 0; i < alleles.size(); ++i) {
                cout << "\t" << alleles[i];
            }
            cout << "\t" << minorAlleleFrequency(alleles) << endl;
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
        //cout << "Total number of segregating sites: " << totalSites << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }


    return 0;
}
