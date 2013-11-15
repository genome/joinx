#pragma once

#include <boost/unordered_map.hpp>

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

class FastaIndex {
public: // types
    struct Entry {
        Entry();

        explicit Entry(std::string const& data);

        std::string name;
        size_t len;
        size_t offset;
        size_t lineBasesLength;
        size_t lineLength;
    };

public: // functions

    FastaIndex();

    explicit FastaIndex(std::istream& in);

    std::vector<std::string> const& sequenceOrder() const;

    Entry const* entry(std::string const& name) const;

    void save(std::ostream& out);


protected: // data
    std::vector<std::string> _sequenceOrder;
    boost::unordered_map<std::string, Entry> _entries;
    friend class FastaIndexGenerator;
};
