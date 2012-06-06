#pragma once

#include "common/namespaces.hpp"

#include <cstdint>
#include <ostream>
#include <set>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class Entry;
class GenotypeCall {
public:
    typedef std::vector<uint32_t>::size_type size_type;
    typedef std::vector<uint32_t>::const_iterator const_iterator;

    static GenotypeCall Null;

    GenotypeCall();
    explicit GenotypeCall(const std::string& call);

    bool empty() const;
    size_type size() const;
    const_iterator begin() const;
    const_iterator end() const;

    const uint32_t& operator[](size_type idx) const;
    bool operator==(const GenotypeCall& rhs) const;
    bool operator!=(const GenotypeCall& rhs) const;
    bool operator<(const GenotypeCall& rhs) const;

    bool phased() const;
    bool heterozygous() const;
    bool homozygous() const;
    bool diploid() const;
    bool reference() const;

    const std::vector<uint32_t>& indices() const;
    const std::string& string() const;
    const std::set<uint32_t>& indexSet() const;


protected:
    bool _phased;
    std::vector<uint32_t> _indices;
    std::set<uint32_t> _indexSet;
    std::string _string;
};

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, Vcf::GenotypeCall const& gt);
