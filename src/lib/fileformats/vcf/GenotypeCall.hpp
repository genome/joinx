#pragma once

#include "common/namespaces.hpp"
#include "common/cstdint.hpp"
#include "common/RelOps.hpp"

#include <ostream>
#include <set>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

struct GenotypeIndex : ValueBasedRelOps<GenotypeIndex>  {
    typedef uint32_t value_type;

    static GenotypeIndex Null;

    GenotypeIndex()
        : value(Null.value)
    {}

    GenotypeIndex(value_type value)
        : value(value)
    {}

    bool null() const {
        return *this == Null;
    }

    value_type value;
};


class GenotypeCall {
public:
    typedef std::vector<GenotypeIndex>::size_type size_type;
    typedef std::vector<GenotypeIndex>::const_iterator const_iterator;

    static GenotypeCall Null;

    GenotypeCall();
    explicit GenotypeCall(const std::string& call);

    bool empty() const;
    bool null() const;
    size_type size() const;
    const_iterator begin() const;
    const_iterator end() const;

    const GenotypeIndex& operator[](size_type idx) const;
    bool operator==(const GenotypeCall& rhs) const;
    bool operator!=(const GenotypeCall& rhs) const;
    bool operator<(const GenotypeCall& rhs) const;

    bool phased() const;
    bool heterozygous() const;
    bool homozygous() const;
    bool diploid() const;
    bool reference() const;

    const std::vector<GenotypeIndex>& indices() const;
    const std::string& string() const;
    const std::set<GenotypeIndex>& indexSet() const;


protected:
    bool _phased;
    std::vector<GenotypeIndex> _indices;
    std::set<GenotypeIndex> _indexSet;
    std::string _string;
};

std::ostream& operator<<(std::ostream& os, GenotypeIndex const& gtidx);
std::ostream& operator<<(std::ostream& os, GenotypeCall const& gt);

END_NAMESPACE(Vcf)
