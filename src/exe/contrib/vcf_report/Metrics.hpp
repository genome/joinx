#pragma once

#include "common/namespaces.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"

#include <map>
#include <vector>

BEGIN_NAMESPACE(Metrics)

class EntryMetrics {
public:
    EntryMetrics();
    void processEntry(Vcf::Entry& entry);
    void calculateGenotypeDistribution(Vcf::Entry& entry);
    void calculateAllelicDistribution();
    void calculateMutationSpectrum(Vcf::Entry& entry);
    bool singleton(const Vcf::GenotypeCall& geno);
    double minorAlleleFrequency();

    const std::map<std::string,uint32_t>& mutationSpectrum() const;
    const std::map<std::string,uint32_t>& singletonMutationSpectrum() const;
    const std::map<Vcf::GenotypeCall,uint32_t>& genotypeDistribution() const;
    const std::vector<uint32_t>& allelicDistribution() const;
    
protected:
    std::map<std::string,uint32_t> _mutationSpectrum;
    std::map<std::string,uint32_t> _singletonMutationSpectrum;
    std::map<Vcf::GenotypeCall,uint32_t> _genotypeDistribution;
    std::vector<uint32_t> _allelicDistribution;
};

class SampleMetrics {
public:
    SampleMetrics();
    void processEntry(Vcf::Entry& e, EntryMetrics& m);
    uint32_t numHetVariants(uint32_t index);
    uint32_t numHomVariants(uint32_t index);
    uint32_t numRefCalls(uint32_t index);
    uint32_t numFilteredCalls(uint32_t index);
    uint32_t numMissingCalls(uint32_t index);
    uint32_t numNonDiploidCalls(uint32_t index);
    uint32_t numSingletonVariants(uint32_t index);
    uint32_t numVeryRareVariants(uint32_t index);
    uint32_t numRareVariants(uint32_t index);
    uint32_t numCommonVariants(uint32_t index);
    uint32_t numTransitions(uint32_t index);
    uint32_t numTransversions(uint32_t index);
    double transitionTransversionRatio(uint32_t index);
protected:
    //per-sample metrics
    std::vector<uint32_t> _perSampleHetVariants;
    std::vector<uint32_t> _perSampleHomVariants;
    std::vector<uint32_t> _perSampleRefCall;
    std::vector<uint32_t> _perSampleFilteredCall;
    std::vector<uint32_t> _perSampleMissingCall;
    std::vector<uint32_t> _perSampleNonDiploidCall;
    std::vector<uint32_t> _perSampleSingletons;
    std::vector<uint32_t> _perSampleVeryRareVariants;
    std::vector<uint32_t> _perSampleRareVariants;
    std::vector<uint32_t> _perSampleCommonVariants;
    std::vector<uint32_t> _perSampleDbSnp;
    std::vector<map<std::string,uint32_t>> _perSampleMutationSpectrum;

};


END_NAMESPACE(Metrics)
