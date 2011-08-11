#pragma once

#include <string>

class InputStream;

enum FileType {
    BED,
    VCF,
    UNKNOWN
};

FileType inferFileType(InputStream& in);
