#pragma once

#include "common/intconfig.hpp"

#include <string>
#include <map>

class BasesFile;
class Sequence;

class ReferenceSequence {
public:
    ReferenceSequence(const std::string& dataDir);
    ~ReferenceSequence();

    Sequence lookup(const std::string& chrom, uint64_t start, uint64_t end);

protected:
    std::string getPathForChrom(const std::string& chrom) const;
    BasesFile* getFileForChrom(const std::string& chrom);

protected:
    typedef std::map<std::string, BasesFile*> MapType;
    std::string _dataDir;
    MapType _files;
};
