#pragma once

#include "common/Sequence.hpp"

#include <cstdint>
#include <string>
#include <map>
#include <fstream>


class BasesFile {
public:
    BasesFile(const std::string& path);

    const std::string& path() const;
    Sequence lookup(uint64_t start, uint64_t end);

protected:
    std::string _path;
    std::ifstream _file;
};

inline const std::string& BasesFile::path() const {
    return _path;
}
