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

EntryMetrics::EntryMetrics()
    : _maxGtIdx(0)
{
}

void EntryMetrics::calculateGenotypeDistribution(Vcf::Entry& entry) {
    // convenience
    auto const& sd = entry.sampleData();
    auto const& fmt = sd.format();

    uint64_t offset = distance(fmt.begin(), find_if(fmt.begin(), fmt.end(), bind(&customTypeIdMatches, "FT", _1)));
    if (offset == fmt.size()) {
        cerr << "No FT tag for call " << entry.chrom() << "\t" << entry.pos() << endl;
    }

    for (auto i = sd.begin(); i != sd.end(); ++i) {
        auto const& sampleIdx = i->first;
        auto const& values = i->second;
        if (values.size() > offset) {
            const std::string *filter(values[offset].get<std::string>(0));
            if (filter != 0 && *filter != "PASS")
                continue;
        }

        //if no FT then we assume all have passed :-(
        Vcf::GenotypeCall const& gt = entry.sampleData().genotype(sampleIdx);
        if(gt.size() == 0) {
            continue;
        }

        if (!gt.diploid()) {
            //anything but diploid is not supported until we understand a bit better how to represent them
            cerr << "Non-diploid genotype for sample " << entry.header().sampleNames()[sampleIdx] << " skipped at position " << entry.chrom() << "\t" << entry.pos() << endl;
            continue;
        }
        else {
            ++_genotypeDistribution[&gt]; //probably ok since should use default contructor of new element before adding 1;
            _maxGtIdx = std::max(_maxGtIdx, *gt.indexSet().rbegin());
        }
    }
}

bool EntryMetrics::singleton(const Vcf::GenotypeCall* geno) const {
    auto iter = _genotypeDistribution.find(geno);
    return iter != _genotypeDistribution.end() && iter->second == 1;
}

void EntryMetrics::calculateAllelicDistribution() {
    _allelicDistribution.resize(_maxGtIdx + 1);
    for( auto i = _genotypeDistribution.begin(); i != _genotypeDistribution.end(); ++i) {
        for(auto j = i->first->begin(); j != i->first->end(); ++j) {
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
        for(auto i = geno->first->indexSet().begin(); i != geno->first->indexSet().end(); ++i) {
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

double EntryMetrics::minorAlleleFrequency() const {
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

const std::map<Vcf::GenotypeCall const*,uint32_t>& EntryMetrics::genotypeDistribution() const {
    return _genotypeDistribution;
}

const std::vector<uint32_t>& EntryMetrics::allelicDistribution() const {
    return _allelicDistribution;
}

SampleMetrics::SampleMetrics(size_t sampleCount)
    : _totalSites(0)
    , _perSampleHetVariants(sampleCount, 0)
    , _perSampleHomVariants(sampleCount, 0)
    , _perSampleRefCall(sampleCount, 0)
    , _perSampleFilteredCall(sampleCount, 0)
    , _perSampleCalls(sampleCount, 0)
    , _perSampleNonDiploidCall(sampleCount, 0)
    , _perSampleSingletons(sampleCount, 0)
    , _perSampleVeryRareVariants(sampleCount, 0)
    , _perSampleRareVariants(sampleCount, 0)
    , _perSampleCommonVariants(sampleCount, 0)
    , _perSampleDbSnp(sampleCount, 0)
    , _perSampleMutationSpectrum(sampleCount)
{
}

void SampleMetrics::processEntry(Vcf::Entry& e, EntryMetrics& m) {
    ++_totalSites;

    // convenience
    auto const& sd = e.sampleData();
    auto const& fmt = sd.format();
    uint64_t offset = distance(fmt.begin(), find_if(fmt.begin(), fmt.end(), bind(&customTypeIdMatches, "FT", _1)));

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

    for (auto i = sd.begin(); i != sd.end(); ++i) {
        auto const& sampleIdx = i->first;
        auto const& values = i->second;

        if (values.size() > offset) {
            const std::string *filter(values[offset].get<std::string>(0));
            if (filter != 0 && *filter != "PASS") {
                ++_perSampleFilteredCall[sampleIdx];
                continue;
            }
        }

        //if no FT then we assume all have passed :-(
        Vcf::GenotypeCall const& gt = e.sampleData().genotype(sampleIdx);
        if(gt.size() == 0) {
            //++_perSampleMissingCall[sampleIdx];
            continue;
        }

        ++_perSampleCalls[sampleIdx];

        if (!gt.diploid()) {
            //anything but diploid is not supported until we understand a bit better how to represent them
            cerr << "Non-diploid genotype for sample " << e.header().sampleNames()[sampleIdx] << " skipped at position " << e.chrom() << "\t" << e.pos() << endl;
            ++_perSampleNonDiploidCall[sampleIdx];
            continue;
        }
        else {
            if(gt.reference()) {
                ++_perSampleRefCall[sampleIdx];
                continue;
            }
            else if(gt.heterozygous()) {
                ++_perSampleHetVariants[sampleIdx];
            }
            else {
                ++_perSampleHomVariants[sampleIdx];
            }

            double maf = m.minorAlleleFrequency();
            if(m.singleton(&gt)) {
                ++_perSampleSingletons[sampleIdx];
            }
            else if(maf < 0.01) {
                ++_perSampleVeryRareVariants[sampleIdx];
            }
            else if(maf >= 0.01 && maf < 0.05) {
                ++_perSampleRareVariants[sampleIdx];
            }
            else {
                ++_perSampleCommonVariants[sampleIdx];
            }

            if(canCalcSpectrum) {
                for(auto j = gt.indexSet().begin(); j != gt.indexSet().end(); ++j) {
                    if(*j == 0) //it's ref
                        continue;
                    std::string variant( e.alt()[*j - 1] );
                    if (variant.size() != 1)
                        throw runtime_error(str(format("Invalid variant for ref entry %1%: %2%") %ref %variant));
                    toupper(variant[0],loc);
                    if(complement)
                        variant = Sequence::reverseComplement(variant);
                    //now have mutation class so go ahead and record
                    _perSampleMutationSpectrum[sampleIdx](ref[0], variant[0]) += 1;
                }
            }
        }
    }
}

uint32_t SampleMetrics::numHetVariants(uint32_t index) const {
    return _perSampleHetVariants[index];
}

uint32_t SampleMetrics::numHomVariants(uint32_t index) const {
    return _perSampleHomVariants[index];
}

uint32_t SampleMetrics::numRefCalls(uint32_t index) const {
    return _perSampleRefCall[index];
}
uint32_t SampleMetrics::numFilteredCalls(uint32_t index) const {
    return _perSampleFilteredCall[index];
}
uint32_t SampleMetrics::numMissingCalls(uint32_t index) const {
    return _totalSites - _perSampleCalls[index] - _perSampleFilteredCall[index];
}
uint32_t SampleMetrics::numNonDiploidCalls(uint32_t index) const {
    return _perSampleNonDiploidCall[index];
}
uint32_t SampleMetrics::numSingletonVariants(uint32_t index) const {
    return _perSampleSingletons[index];
}
uint32_t SampleMetrics::numVeryRareVariants(uint32_t index) const {
    return _perSampleVeryRareVariants[index];
}
uint32_t SampleMetrics::numRareVariants(uint32_t index) const {
    return _perSampleRareVariants[index];
}
uint32_t SampleMetrics::numCommonVariants(uint32_t index) const {
    return _perSampleCommonVariants[index];
}

MutationSpectrum const& SampleMetrics::mutationSpectrum(uint32_t index) const {
    return _perSampleMutationSpectrum[index];
}

END_NAMESPACE(Metrics)

