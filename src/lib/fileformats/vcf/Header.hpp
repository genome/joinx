#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Map.hpp"
#include "namespace.hpp"

VCF_NAMESPACE_BEGIN

class Header {
public:
    typedef std::vector<Map> Category;
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
    const std::set<std::string>& categories() const;
    const Category& category(const std::string& name) const;
    const Map* categoryItem(const std::string& catName, const std::string& id) const;
    const std::vector<std::string>& sampleNames() const;

    // throws when sampleName is not found
    unsigned sampleIndex(const std::string& sampleName);

    // will throw if the header is invalid
    void assertValid() const;

protected:
    void parseHeaderLine(const std::string& line);

protected:
    std::vector<RawLine> _metaInfoLines;
    std::set<std::string> _categoryNames;
    std::map<std::string, Category> _categories;
    std::vector<std::string> _sampleNames;
    Category _empty; // so we can return an empty result by constref
    bool _headerSeen;
};

inline const std::vector<Header::RawLine>& Header::metaInfoLines() const {
    return _metaInfoLines;
}

inline const std::set<std::string>& Header::categories() const {
    return _categoryNames;
}

inline const Header::Category& Header::category(const std::string& name) const {
    auto iter = _categories.find(name);
    if (iter == _categories.end())
        return _empty;
    return iter->second;
}

inline const std::vector<std::string>& Header::sampleNames() const {
    return _sampleNames;
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h);
