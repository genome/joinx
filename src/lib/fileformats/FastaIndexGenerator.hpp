#pragma once

#include "FastaIndex.hpp"

#include <memory>

class FastaIndexGenerator {
public:
    typedef FastaIndex::Entry Entry;

    FastaIndexGenerator(char const* data, size_t len);

    std::unique_ptr<FastaIndex> generate();

protected:
    void extractName(Entry& e);
    void countLines(Entry& e);

protected:
    char const* _beg;
    char const* _pos;
    char const* _end;
};

