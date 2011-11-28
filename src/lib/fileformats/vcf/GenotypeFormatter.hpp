#pragma once

#include "common/namespaces.hpp"

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class CustomValue;
class Entry;
class Header;

class DisjointGenotypesError : public std::runtime_error {
public:
    DisjointGenotypesError(const std::string& what)
        : std::runtime_error(what)
    {}
};

class GenotypeFormatter {
public:
    GenotypeFormatter(const Header* header, const std::vector<std::string>& alleles);

    std::vector<CustomValue> process(
        const std::vector<std::string>& fields,
        const Entry* e,
        uint32_t sampleIdx,
        const std::vector<size_t>& alleleIndices
        ) const;

    void merge(
        bool overridePreviousValues,
        std::vector<CustomValue>& previousValues,
        const std::vector<std::string>& fields,
        const Entry* e,
        uint32_t sampleIdx,
        const std::vector<size_t>& alleleIndices
        ) const;
        

    std::string renumberGT(
        const Entry* e,
        uint32_t sampleIdx,
        const std::vector<size_t>& alleleIndices) const;

    static bool areGenotypesDisjoint(const std::string& gt1, const std::string& gt2);

protected:
    const Header* _header;
    const std::vector<std::string>& _alleles;
};

END_NAMESPACE(Vcf)
