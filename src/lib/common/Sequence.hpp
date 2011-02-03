#pragma once

#include "common/intconfig.hpp"

#include <string>
#include <iostream>

class Sequence {
public:
    static std::string reverseComplement(const std::string& data);

    Sequence();
    Sequence(const std::string& data);
    Sequence(char data);
    Sequence(std::istream& s, uint64_t count);

    Sequence& operator=(const std::string& rhs);

    bool null() const;
    bool empty() const;
    const std::string& data() const;
    const std::string& reverseComplementData() const;

    bool operator==(const Sequence& rhs) const;
    bool operator!=(const Sequence& rhs) const;

protected:
    std::string _data;
    mutable std::string _reverseComplement;
};

inline bool Sequence::empty() const {
    return _data.empty();
}

inline const std::string& Sequence::data() const {
    return _data;
}

inline Sequence& Sequence::operator=(const std::string& rhs) {
    _data = rhs;
    return *this;
}

inline bool Sequence::null() const {
    return empty() || _data == "-" || _data == "0";
}

inline bool Sequence::operator==(const Sequence& rhs) const {
    return _data == rhs._data;
}

inline bool Sequence::operator!=(const Sequence& rhs) const {
    return !(*this == rhs);
}

