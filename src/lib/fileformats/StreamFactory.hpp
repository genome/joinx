#pragma once

#include "TypedStream.hpp"

#include <memory>

class InputStream;

template<typename ValueClass, typename Extractor>
class StreamFactory {
public:
    typedef ValueClass ValueType;
    typedef TypedStream<ValueType, Extractor> StreamType;
    typedef std::shared_ptr<StreamType> StreamPtr;

    StreamFactory(Extractor& extractor)
        : _extractor(extractor)
    {}

    StreamPtr open(InputStream& in) {
        return StreamPtr(new StreamType(_extractor, in));
    }

protected:
    Extractor& _extractor;
};
