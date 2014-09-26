#pragma once

#include "TypedStream.hpp"
#include "Bed.hpp"

#include <string>

typedef TypedStream<BedParser> BedReader;

BedReader::ptr openBed(InputStream& in, int maxExtraFields = -1);
