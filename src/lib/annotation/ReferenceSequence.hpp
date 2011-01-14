#pragma once

#include "common/intconfig.hpp"

#include <string>
#include <map>
#include <fstream>

class Sequence;

class ReferenceSequence {
public:
    ReferenceSequence(const std::string& dataDir);
    ~ReferenceSequence();

    Sequence lookup(const std::string& chrom, uint64_t start, uint64_t end);

protected:
    std::string getPathForChrom(const std::string& chrom) const;
    std::ifstream* getFileForChrom(const std::string& chrom);

protected:
    typedef std::map<std::string, std::ifstream*> MapType;
    std::string _dataDir;
    MapType _files;
};
