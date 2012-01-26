#pragma once

#include "common/MutationSpectrum.hpp"
#include "common/namespaces.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"

#include <array>
#include <cstddef>
#include <map>
#include <vector>

class MutationSpectrum;

BEGIN_NAMESPACE(Metrics)

class EntryMetrics {
public:
    EntryMetrics();
    void processEntry(Vcf::Entry& entry);
    void calculateGenotypeDistribution(Vcf::Entry& entry);
    void calculateAllelicDistribution();
    void calculateAllelicDistributionBySample();
    void calculateMutationSpectrum(Vcf::Entry& entry);
    double minorAlleleFrequency() const;
    const std::vector<double> alleleFrequencies() const;

    const MutationSpectrum& mutationSpectrum() const;
    const MutationSpectrum& singletonMutationSpectrum() const;
    const std::map<Vcf::GenotypeCall const,uint32_t>& genotypeDistribution() const;
    const std::vector<uint32_t>& allelicDistribution() const;
    const std::vector<uint32_t>& allelicDistributionBySample() const;
    
    bool singleton(const Vcf::GenotypeCall* geno) const;

protected:
    uint32_t _maxGtIdx;
    MutationSpectrum _mutationSpectrum;
    MutationSpectrum _singletonMutationSpectrum;
    std::map<Vcf::GenotypeCall const,uint32_t> _genotypeDistribution;
    std::vector<uint32_t> _allelicDistribution;
    std::vector<uint32_t> _allelicDistributionBySample;
};

class SampleMetrics {
public:
    SampleMetrics(size_t sampleCount);
    void processEntry(Vcf::Entry& e, EntryMetrics& m);
    uint32_t numHetVariants(uint32_t index) const;
    uint32_t numHomVariants(uint32_t index) const;
    uint32_t numRefCalls(uint32_t index) const;
    uint32_t numFilteredCalls(uint32_t index) const;
    uint32_t numMissingCalls(uint32_t index) const;
    uint32_t numNonDiploidCalls(uint32_t index) const;
    uint32_t numSingletonVariants(uint32_t index) const;
    uint32_t numVeryRareVariants(uint32_t index) const;
    uint32_t numRareVariants(uint32_t index) const;
    uint32_t numCommonVariants(uint32_t index) const;
    MutationSpectrum const& mutationSpectrum(uint32_t index) const;

protected:
    uint64_t _totalSites;
    //per-sample metrics
    std::vector<uint32_t> _perSampleHetVariants;
    std::vector<uint32_t> _perSampleHomVariants;
    std::vector<uint32_t> _perSampleRefCall;
    std::vector<uint32_t> _perSampleFilteredCall;
    std::vector<uint32_t> _perSampleCalls;
    std::vector<uint32_t> _perSampleNonDiploidCall;
    std::vector<uint32_t> _perSampleSingletons;
    std::vector<uint32_t> _perSampleVeryRareVariants;
    std::vector<uint32_t> _perSampleRareVariants;
    std::vector<uint32_t> _perSampleCommonVariants;
    std::vector<uint32_t> _perSampleDbSnp;
    std::vector<MutationSpectrum> _perSampleMutationSpectrum;
};


END_NAMESPACE(Metrics)
