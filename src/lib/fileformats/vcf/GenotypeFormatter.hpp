#pragma once

#include "namespace.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

class CustomValue;
class Entry;
class Header;

class GenotypeFormatter {
public:
    typedef std::map<std::string, uint32_t> AlleleMap;

    GenotypeFormatter(const Header* header, const AlleleMap& alleles);

    std::vector<CustomValue> process(
        const std::vector<std::string>& fields,
        const Entry* e,
        uint32_t idx) const;

    std::string renumberGT(const Entry* e, uint32_t idx) const;

protected:
    const Header* _header;
    const AlleleMap& _alleles;
};

VCF_NAMESPACE_END
