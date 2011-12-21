#include "Metrics.hpp"
#include "fileformats/vcf/Header.hpp"
#include "common/Sequence.hpp"

#include <locale>
#include <string>
#include <boost/format.hpp>
#include <stdexcept>
#include <functional>
#include <numeric>

using boost::format;
using namespace std;
using namespace std::placeholders;

BEGIN_NAMESPACE(Metrics)
namespace {
bool customTypeIdMatches(string const& id, Vcf::CustomType const* type) {
    return type && type->id() == id;
}

bool minorAlleleSort (int i,int j) { return (i != 0 && i<j); }
}

EntryMetrics::EntryMetrics() {
//empty for now
}

void EntryMetrics::calculateGenotypeDistribution(Vcf::Entry& entry) {
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
        if (!gt.diploid()) {
            //anything but diploid is not supported until we understand a bit better how to represent them
            cerr << "Non-diploid genotype for sample " << entry.header().sampleNames()[i] << " skipped at position " << entry.chrom() << "\t" << entry.pos() << endl;
            continue;
        }
        else {
            _genotypeDistribution[gt]++; //probably ok since should use default contructor of new element before adding 1;
        }
    }
}

bool EntryMetrics::singleton(const Vcf::GenotypeCall& geno) {
    return _genotypeDistribution[geno] == 1;
}

void EntryMetrics::calculateAllelicDistribution() {
    for( auto i = _genotypeDistribution.begin(); i != _genotypeDistribution.end(); ++i) {
        for(auto j = (*i).first.begin(); j != (*i).first.end(); ++j) {
            if(*j >= _allelicDistribution.size()) {
                _allelicDistribution.resize(*j + 1);
            }
            _allelicDistribution[(*j)] += (*i).second;
        }
    }
}

void EntryMetrics::calculateMutationSpectrum(Vcf::Entry& entry) {
    if(entry.ref().size() != 1) {
        throw runtime_error(str(format("Unable to calculate the mutation spectrum on non-snp positions at %1%\t%2%\t%3%") % entry.chrom() % entry.pos() % entry.ref()));
    }
    locale loc;
    std::string ref(entry.ref());
    toupper(ref[0],loc);
    bool complement = false;
    if(ref == "G" || ref == "T") {
        complement = true;
        ref = Sequence::reverseComplement(ref);
    }
    for(auto geno = _genotypeDistribution.begin(); geno != _genotypeDistribution.end(); ++geno) {
        std::string mutationClass = ref + "->"; //this feels a little too perly. too bad.
        for(auto i = (*geno).first.indexSet().begin(); i != (*geno).first.indexSet().end(); ++i) {
            if(*i == 0) //it's ref
                continue;
            std::string variant( entry.alt()[*i - 1] );
            //cerr << variant;
            toupper(variant[0],loc);

            if(complement) {
                variant = Sequence::reverseComplement(variant);
            }
            mutationClass += variant;
            if(this->singleton((*geno).first)) {
               _singletonMutationSpectrum[mutationClass] += (*geno).second; //this is probably not right as it would double count homozygotes
            }
            else {
                _mutationSpectrum[mutationClass] += (*geno).second;
            }
        }
    }
}

double EntryMetrics::minorAlleleFrequency() {
    if(!_allelicDistribution.empty()) {
        uint32_t totalAlleles = accumulate(_allelicDistribution.begin(), _allelicDistribution.end(), 0);
        return (double) *min_element(_allelicDistribution.begin(), _allelicDistribution.end(), minorAlleleSort) / totalAlleles; 
    }
    else {
        throw runtime_error("Unable to calculate minorAlleleFrequency if the allelic distribution is empty");
    }
}

void EntryMetrics::processEntry(Vcf::Entry& entry) {
    calculateGenotypeDistribution(entry);
    calculateAllelicDistribution();
    calculateMutationSpectrum(entry);
}

const std::map<std::string,uint32_t>& EntryMetrics::mutationSpectrum() const {
    return _mutationSpectrum;
}

const std::map<std::string,uint32_t>& EntryMetrics::singletonMutationSpectrum() const {
    return _singletonMutationSpectrum;
}

const std::map<Vcf::GenotypeCall,uint32_t>& EntryMetrics::genotypeDistribution() const {
    return _genotypeDistribution;
}

const std::vector<uint32_t>& EntryMetrics::allelicDistribution() const {
    return _allelicDistribution;
}

SampleMetrics::SampleMetrics() {
}

void SampleMetrics::processEntry(Vcf::Entry& e, EntryMetrics& m) {
    uint32_t size = e.header().sampleCount();
    _perSampleHetVariants.resize(size);
    _perSampleHomVariants.resize(size);
    _perSampleRefCall.resize(size);
    _perSampleFilteredCall.resize(size);
    _perSampleMissingCall.resize(size);
    _perSampleNonDiploidCall.resize(size);
    _perSampleSingletons.resize(size);
    _perSampleVeryRareVariants.resize(size);
    _perSampleRareVariants.resize(size);
    _perSampleCommonVariants.resize(size);
    _perSampleDbSnp.resize(size);
    _perSampleMutationSpectrum.resize(size);

    //now everything is the right size
    auto it = find_if(e.formatDescription().begin(), e.formatDescription().end(), bind(&customTypeIdMatches, "FT", _1));

    int32_t offset;
    if (it != e.formatDescription().end()) {
        offset = distance(e.formatDescription().begin(), it);
    }
    else {
        offset = -1;
    }

    locale loc;
    std::string ref(e.ref());   //for mutation spectrum

    bool canCalcSpectrum = false;
    bool complement = false;

    if(e.ref().size() == 1) {
        canCalcSpectrum = true;
        toupper(ref[0],loc);
        if(ref == "G" || ref == "T") {
            complement = true;
            ref = Sequence::reverseComplement(ref);
        }
    }
    for (uint32_t i = 0; i < e.header().sampleCount(); ++i) {
        if (offset >= 0) {
            const std::string *filter;
            if (e.sampleData(i,"FT") != NULL && (filter = e.sampleData(i,"FT")->get<std::string>(0)) != NULL &&  *filter != "PASS") {
                _perSampleFilteredCall[i]++;
                continue;
            }
        }
        //if no FT then we assume all have passed :-(
        Vcf::GenotypeCall gt = e.genotypeForSample(i);
        if(gt.size() == 0) {
            _perSampleMissingCall[i]++;
            continue;
        }
        if (!gt.diploid()) {
            //anything but diploid is not supported until we understand a bit better how to represent them
            cerr << "Non-diploid genotype for sample " << e.header().sampleNames()[i] << " skipped at position " << e.chrom() << "\t" << e.pos() << endl;
            _perSampleNonDiploidCall[i]++;
            continue;
        }
        else {
            if(gt.reference()) {
                _perSampleRefCall[i]++;
                continue;
            }
            else if(gt.heterozygous()) {
                _perSampleHetVariants[i]++;
            }
            else {
                _perSampleHomVariants[i]++;
            }

            double maf = m.minorAlleleFrequency();
            if(m.singleton(gt)) {
                _perSampleSingletons[i]++;
            }
            else if(maf < 0.01) {
                _perSampleVeryRareVariants[i]++;
            }
            else if(maf >= 0.01 && maf < 0.05) {
                _perSampleRareVariants[i]++;
            }
            else {
                _perSampleCommonVariants[i]++;
            }

            if(canCalcSpectrum) {
                std::string mutationClass = ref + "->"; //this feels a little too perly. too bad.
                for(auto j = gt.indexSet().begin(); j != gt.indexSet().end(); ++j) {
                    if(*j == 0) //it's ref
                        continue;
                    std::string variant( e.alt()[*j - 1] );
                    toupper(variant[0],loc);
                    if(complement) {
                        variant = Sequence::reverseComplement(variant);
                    }
                    mutationClass += variant;
                    //now have mutation class so go ahead and record
                    _perSampleMutationSpectrum[i][mutationClass] += 1;
                }
            }
        }
    }
}

uint32_t SampleMetrics::numHetVariants(uint32_t index) {
    return _perSampleHetVariants[index];
}


uint32_t SampleMetrics::numHomVariants(uint32_t index) {
    return _perSampleHomVariants[index];
}

uint32_t SampleMetrics::numRefCalls(uint32_t index) {
    return _perSampleRefCall[index];
}
uint32_t SampleMetrics::numFilteredCalls(uint32_t index) {
    return _perSampleFilteredCall[index];
}
uint32_t SampleMetrics::numMissingCalls(uint32_t index) {
    return _perSampleMissingCall[index];
}
uint32_t SampleMetrics::numNonDiploidCalls(uint32_t index) {
    return _perSampleNonDiploidCall[index];
}
uint32_t SampleMetrics::numSingletonVariants(uint32_t index) {
    return _perSampleSingletons[index];
}
uint32_t SampleMetrics::numVeryRareVariants(uint32_t index) {
    return _perSampleVeryRareVariants[index];
}
uint32_t SampleMetrics::numRareVariants(uint32_t index) {
    return _perSampleRareVariants[index];
}
uint32_t SampleMetrics::numCommonVariants(uint32_t index) {
    return _perSampleCommonVariants[index];
}
uint32_t SampleMetrics::numTransitions(uint32_t index) {
    std::map<std::string, uint32_t> spectrum = _perSampleMutationSpectrum[index];
    return spectrum["A->G"] + spectrum["C->T"];
}

uint32_t SampleMetrics::numTransversions(uint32_t index) {
    std::map<std::string, uint32_t> spectrum = _perSampleMutationSpectrum[index];
    return spectrum["A->C"] + spectrum["A->T"] + spectrum["C->A"] + spectrum["C->G"];
}

double SampleMetrics::transitionTransversionRatio(uint32_t index) {
    std::map<std::string, uint32_t> spectrum = _perSampleMutationSpectrum[index];
    double transitions = spectrum["A->G"] + spectrum["C->T"];
    double transversions = spectrum["A->C"] + spectrum["A->T"] + spectrum["C->A"] + spectrum["C->G"];
    return transitions/transversions;
}

END_NAMESPACE(Metrics)

