#pragma once

#include "common/namespaces.hpp"

#include <map>
#include <ostream>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class SampleTag {
public:
    SampleTag();
    explicit SampleTag(std::string const& raw);

    void toStream(std::ostream& s) const;
    std::string toString() const;
    void set(std::string const& name, std::string const& value);
    std::string const& id() const;

protected:
    std::map<std::string, std::string> _fields;
    std::vector<std::string> _fieldOrder;
};

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, Vcf::SampleTag const& sampleTag);
