#pragma once

#include "common/namespaces.hpp"

#include <map>
#include <ostream>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class SampleTag {
public:
    typedef std::pair<std::string, std::string> PairType;
    typedef std::vector<PairType> FieldsType;

    SampleTag();
    explicit SampleTag(std::string const& raw);

    void toStream(std::ostream& s) const;
    std::string toString() const;
    void set(std::string const& name, std::string const& value);
    std::string const* get(std::string const& key) const;
    std::string const& id() const;
    

protected:
    FieldsType _fields;
    std::map<std::string, size_t> _fieldIndex;
};

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, Vcf::SampleTag const& sampleTag);
