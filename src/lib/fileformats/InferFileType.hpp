#pragma once

#include <string>

class InputStream;

enum FileType {
    BED,
    VCF,
    CHROMPOS,
    EMPTY,
    UNKNOWN
};

FileType inferFileType(InputStream& in);
