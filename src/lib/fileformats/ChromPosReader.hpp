#pragma once

#include "TypedStream.hpp"
#include "ChromPos.hpp"

typedef TypedStream<DefaultParser<ChromPos>> ChromPosReader;
ChromPosReader::ptr openChromPos(InputStream& in);
