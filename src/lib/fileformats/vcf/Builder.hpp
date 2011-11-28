#pragma once

#include "common/namespaces.hpp"

#include <functional>
#include <vector>
#include <string>

BEGIN_NAMESPACE(Vcf)

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
};

END_NAMESPACE(Vcf)
