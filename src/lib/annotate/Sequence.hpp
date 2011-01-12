#pragma once

#include <string>


class Sequence {
public:
    static std::string reverseComplement(const std::string& data);

    Sequence();
    Sequence(const std::string& data);

    Sequence& operator=(const std::string& rhs);

    bool empty() const;
    const std::string& data() const;
    const std::string& reverseComplementData() const;


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


