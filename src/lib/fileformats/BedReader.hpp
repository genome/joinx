#pragma once

#include "TypedStream.hpp"
#include "Bed.hpp"

#include <boost/function.hpp>

#include <functional>
#include <string>

namespace {
typedef boost::function<void(const BedHeader*, std::string&, Bed&)> BedExtractor;
typedef TypedStream<Bed, BedExtractor> BedReader;
typedef boost::function<BedReader::ptr(InputStream&)> BedOpenerType;
}

BedReader::ptr openBed(InputStream& in, int maxExtraFields = -1);
