#pragma once

#include <string>

class InputStream;

enum FileType {
    BED,
    VCF,
    EMPTY,
    UNKNOWN
};

FileType inferFileType(InputStream& in);
