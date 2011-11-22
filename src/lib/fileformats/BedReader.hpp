#pragma once

#include "TypedStream.hpp"
#include "Bed.hpp"

#include <functional>
#include <string>

namespace {
typedef std::function<void(const BedHeader*, std::string&, Bed&)> BedExtractor;
typedef TypedStream<Bed, BedExtractor> BedReader;
typedef std::function<BedReader::ptr(InputStream&)> BedOpenerType;
}

BedReader::ptr openBed(InputStream& in, int maxExtraFields = -1);
