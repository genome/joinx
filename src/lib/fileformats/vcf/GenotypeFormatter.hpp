#pragma once

#include "namespace.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

class CustomValue;
class Entry;
class Header;

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

VCF_NAMESPACE_END
