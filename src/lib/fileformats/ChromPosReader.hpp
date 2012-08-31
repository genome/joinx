#pragma once

#include "TypedStream.hpp"
#include "ChromPos.hpp"

#include <functional>
#include <string>

namespace {
typedef std::function<void(const ChromPosHeader*, std::string&, ChromPos&)> ChromPosExtractor;
typedef TypedStream<ChromPos, ChromPosExtractor> ChromPosReader;
typedef std::function<ChromPosReader::ptr(InputStream&)> ChromPosOpenerType;
}

ChromPosReader::ptr openChromPos(InputStream& in);
