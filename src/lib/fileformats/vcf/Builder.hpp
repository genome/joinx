#pragma once

#include "namespace.hpp"

#include <functional>
#include <vector>
#include <string>

VCF_NAMESPACE_BEGIN

class Entry;
class Header;
class MergeStrategy;

class Builder {
public:
    typedef std::function<void(const Entry&)> OutputFunc;
    Builder(const MergeStrategy& mergeStrategy, Header* header, OutputFunc out);
    ~Builder();

    void operator()(const Entry& e);
    void flush();

    static bool canMerge(const Entry& a, const Entry& b);

protected:
    void push(const Entry& e);
    std::vector<Entry>::iterator partition();
    bool canMergeWithMaxRef(const Entry& e) const;
    void output(const Entry* begin, const Entry* end) const;

protected:
    const MergeStrategy& _mergeStrategy;
    Header* _header;
    std::vector<Entry> _entries;
    OutputFunc _out;
    std::string::size_type _maxRefLen;
};

VCF_NAMESPACE_END
