#pragma once

#include "common/namespaces.hpp"
#include "fileformats/FastaReader.hpp"

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
    Builder(
        const MergeStrategy& mergeStrategy,
        Header* header,
        OutputFunc out
        );
    ~Builder();

    void operator()(const Entry& e);
    void operator()(Entry&& e);
    void flush();

protected:
    std::vector<Entry>::iterator partition();
    void output(Entry* begin, Entry* end) const;

    void writeMergedEntry(Entry& e) const;

protected:
    const MergeStrategy& _mergeStrategy;
    Header* _header;
    std::vector<Entry> _entries;
    OutputFunc _out;
};

END_NAMESPACE(Vcf)
