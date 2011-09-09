#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "CustomType.hpp"
#include "namespace.hpp"

VCF_NAMESPACE_BEGIN

class Header {
public:
    typedef std::pair<std::string, std::string> RawLine;

    template<typename T>
    static Header fromStream(T& stream) {
        Header rv;
        std::string line;
        while (stream.peek() == '#' && stream.getline(line))
            rv.add(line);
        return rv;
    }

    Header();

    void add(const std::string& line);
    void merge(const Header& other);

    const std::vector<RawLine>& metaInfoLines() const;
    std::string headerLine() const;
    // infoType/formatType return NULL for non-existing ids
    const CustomType* infoType(const std::string& id) const;
    const CustomType* formatType(const std::string& id) const;
    const std::map<std::string, CustomType>& infoTypes() const;
    const std::map<std::string, CustomType>& formatTypes() const;
    const std::map<std::string, std::string>& filters() const;
    const std::vector<std::string>& sampleNames() const;

    // throws when sampleName is not found
    unsigned sampleIndex(const std::string& sampleName);

    // will throw if the header is invalid
    void assertValid() const;

protected:
    void parseHeaderLine(const std::string& line);

protected:
    std::map<std::string, CustomType> _infoTypes;
    std::map<std::string, CustomType> _formatTypes;
    // filters = name -> description
    std::map<std::string, std::string> _filters;
    std::vector<RawLine> _metaInfoLines;
    std::vector<std::string> _sampleNames;
    bool _headerSeen;
};

inline const CustomType* Header::infoType(const std::string& id) const {
    auto iter = _infoTypes.find(id);
    if (iter == _infoTypes.end());
        return 0;
    return &iter->second;
}

inline const CustomType* Header::formatType(const std::string& id) const {
    auto iter = _formatTypes.find(id);
    if (iter == _formatTypes.end());
        return 0;
    return &iter->second;
}

inline const std::map<std::string, CustomType>& Header::infoTypes() const {
    return _infoTypes;
}

inline const std::map<std::string, CustomType>& Header::formatTypes() const {
    return _formatTypes;
}

inline const std::map<std::string, std::string>& Header::filters() const {
    return _filters;
}

inline const std::vector<Header::RawLine>& Header::metaInfoLines() const {
    return _metaInfoLines;
}

inline const std::vector<std::string>& Header::sampleNames() const {
    return _sampleNames;
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h);
