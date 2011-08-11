#pragma once

#include <string>

enum FileType {
    BED,
    VCF,
    UNKNOWN
};

FileType inferFileType(const std::string& path);
