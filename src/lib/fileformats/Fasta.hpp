#pragma once

#include "FastaIndex.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include <cstddef>
#include <memory>
#include <string>

class Fasta {
public:
    explicit Fasta(std::string const& path);

    // this is useful for testing with with data in memory
    Fasta(
        std::string const& name,
        char const* data,
        size_t len
        );

    size_t seqlen(std::string const& seq) const;
    char sequence(std::string const& seq, size_t pos) const;
    std::string sequence(std::string const& seq, size_t pos, size_t len) const;

    std::string const& name() const;

    FastaIndex const& index() const;

protected:
    std::string _name;
    std::unique_ptr<FastaIndex> _index;
    char const* _data;
    size_t _len;
    std::unique_ptr<boost::iostreams::mapped_file_source> _f;
};
