#pragma once

#include "common/namespaces.hpp"

#include <cstdint>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class GenotypeCall {
public:
    typedef std::vector<uint32_t>::size_type size_type;
    typedef std::vector<uint32_t>::const_iterator const_iterator;

    GenotypeCall();
    explicit GenotypeCall(const std::string& call);

    bool empty() const;
    size_type size() const;
    const_iterator begin() const;
    const_iterator end() const;

    const uint32_t& operator[](size_type idx) const;

    bool phased() const;
    const std::vector<uint32_t>& indices() const;

protected:
    bool _phased;
    std::vector<uint32_t> _indices;
};

END_NAMESPACE(Vcf)
