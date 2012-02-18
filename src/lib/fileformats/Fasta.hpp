#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <cstddef>
#include <memory>
#include <string>

class Fasta {
public:
    class Index;

    explicit Fasta(std::string const& path);

    // this is useful for testing with with data in memory
    Fasta(
        std::string const& name,
        char const* data,
        size_t len
        );

    ~Fasta();

    std::string sequence(std::string const& seq, size_t pos, size_t len);

protected:
    std::string _name;
    std::unique_ptr<Index> _index;
    char const* _data;
    size_t _len;
    std::unique_ptr<boost::iostreams::mapped_file_source> _f;
};
