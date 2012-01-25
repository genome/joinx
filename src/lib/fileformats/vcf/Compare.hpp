#pragma once

#include "common/namespaces.hpp"

#include <cstdint>
#include <map>

BEGIN_NAMESPACE(Vcf)

class Entry;

BEGIN_NAMESPACE(Compare)

struct AltIntersect {
    typedef std::map<uint32_t, uint32_t> ResultType;
    ResultType operator()(Entry const& a, Entry const& b) const;
};

END_NAMESPACE(Compare)
END_NAMESPACE(Vcf)
