#pragma once

#include "namespace.hpp"

#include <boost/format.hpp>

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

VCF_NAMESPACE_BEGIN

// Defines a map class parsed as k1=v1,k2=v2,...
// also remembers the original order of the elements
class Map {
public:
    typedef std::map<std::string, std::string>::size_type size_type;

    Map();
    explicit Map(const std::string& data);

    const std::string& operator[](const std::string& key) const;

    size_type size() const;
    bool empty() const;
    
    template<typename T>
    void insert(const std::string& key, const T& value);
    void remove(const std::string& key);

    const std::vector<std::string> keyOrder() const;

    const std::string& toString() const;

    template<typename T>
    T as(const std::string& key) const;

    bool operator==(const Map& rhs) const;

protected:
    mutable std::string _str;
    std::map<std::string, std::string> _map;
    std::vector<std::string> _keyOrder;
};

inline const std::string& Map::operator[](const std::string& key) const {
   using boost::format;

    auto iter = _map.find(key);
    if (iter == _map.end())
        throw std::runtime_error(str(format("Request for non-existing key in VCF map: %1%") %key));
    return iter->second;
}

template<>
inline void Map::insert<std::string>(const std::string& key, const std::string& value) {
    using namespace std;
    using boost::format;

    auto p = _map.insert(make_pair(key, value));
    if (!p.second)
        throw runtime_error(str(format("Duplicate key in VCF map: %1%") %key));

    _str.clear();
}

template<typename T>
inline void Map::insert(const std::string& key, const T& value) {
    std::stringstream ss;
    ss << value;
    insert<std::string>(key, value);
    _keyOrder.push_back(key);
}

inline Map::size_type Map::size() const {
    return _map.size();
}

inline bool Map::empty() const {
    return _map.empty();
}

inline void Map::remove(const std::string& key) {
    _map.erase(key);
    _str.clear();
}

inline const std::vector<std::string> Map::keyOrder() const {
    return _keyOrder;
}

template<typename T>
inline T Map::as(const std::string& key) const {
    std::stringstream ss(operator[](key));
    T rv;
    ss >> rv;
    return rv; 
}

inline bool Map::operator==(const Map& rhs) const {
    // ignoring order of fields
    return _map == _map;
}

VCF_NAMESPACE_END
