#pragma once

#include "Vcf.hpp"

#include <iostream>
#include <string>

VCF_NAMESPACE_BEGIN

class Reader {
public:
    typedef Entry ValueType;

    Reader(const std::string& streamName, std::istream& in);

    const std::string& name() const;
    const Header& header() const;

    bool eof();
    bool next(Entry& e);
    bool peek(Entry** e);

protected:
    void parseHeader();

protected:
    std::string _streamName;
    Header _header;
    std::istream& _in;
    std::string _buf;
    bool _cached;
    bool _cachedRv;
    Entry _cachedEntry;
};

inline const std::string& Reader::name() const {
    return _streamName;
}

VCF_NAMESPACE_END
