#pragma once

#include "common/namespaces.hpp"

#include <cstddef>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class Entry;
class Header;

class ConsensusFilter {
public:
    ConsensusFilter(double percent, std::string const& filterName, Header const* hdr);

    void apply(Entry& entry, std::vector<size_t> const* counts) const;

protected:
    double _percent;
    std::string _filterName;
    Header const* _header;
};

END_NAMESPACE(Vcf)
