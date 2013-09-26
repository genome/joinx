#pragma once

#include "TypedStream.hpp"
#include "ChromPos.hpp"

#include <boost/function.hpp>

#include <functional>
#include <string>

namespace {
typedef boost::function<void(const ChromPosHeader*, std::string&, ChromPos&)> ChromPosExtractor;
typedef TypedStream<ChromPos, ChromPosExtractor> ChromPosReader;
typedef boost::function<ChromPosReader::ptr(InputStream&)> ChromPosOpenerType;
}

ChromPosReader::ptr openChromPos(InputStream& in);
