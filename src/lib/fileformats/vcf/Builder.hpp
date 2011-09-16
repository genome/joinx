#pragma once

#include "namespace.hpp"

#include <functional>
#include <vector>

VCF_NAMESPACE_BEGIN

class Entry;
class Header;

class Builder {
public:
    typedef std::function<void(const Entry&)> OutputFunc;
    Builder(Header* header, OutputFunc out);

    void operator()(const Entry& e);
    void flush();

    static bool canMerge(const Entry& a, const Entry& b);

protected:
    Header* _header;
    std::vector<Entry> _entries;
    OutputFunc _out;
};

VCF_NAMESPACE_END
