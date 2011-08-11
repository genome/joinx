#pragma once

#include "Entry.hpp"
#include "Header.hpp"
#include "namespace.hpp"
#include "fileformats/InputStream.hpp"

#include <string>

VCF_NAMESPACE_BEGIN

class Reader {
public:
    typedef Entry ValueType;

    Reader(InputStream& in);

    const std::string& name() const;
    const Header& header() const;

    bool eof();
    bool next(Entry& e);
    bool peek(Entry** e);

protected:
    void parseHeader();

protected:
    Header _header;
    InputStream& _in;
    std::string _buf;
    bool _cached;
    bool _cachedRv;
    Entry _cachedEntry;
};

inline const std::string& Reader::name() const {
    return _in.name();
}

VCF_NAMESPACE_END
