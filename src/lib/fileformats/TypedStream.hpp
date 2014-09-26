#pragma once

#include "common/compat.hpp"
#include "io/InputStream.hpp"

#include <boost/format.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


template<typename Extractor>
class TypedStream {
public:
    typedef typename Extractor::ValueType ValueType;
    typedef typename ValueType::HeaderType HeaderType;
    typedef std::unique_ptr<TypedStream<Extractor> > ptr;

    TypedStream(Extractor& extractor, InputStream& in)
        : extractor_(extractor)
        , in_(in)
        , valueCount_(0)
        , cached_(false)
        , cachedRv_(false)
    {
        using boost::format;
        try {
            header_ = HeaderType::fromStream(in_);
        }
        catch (std::exception const& e) {
            throw std::runtime_error(str(format(
                "Error while parsing header of %1%:\n%2%"
                ) % in.name() % e.what()));
        }
    }

    HeaderType const& header() const {
        return header_;
    }

    HeaderType& header() {
        return header_;
    }

    std::string const& name() const;
    bool eof() const;
    bool peek(ValueType** value);
    bool next(ValueType& value);
    uint64_t valueCount() const;
    void checkEof() const;
    uint64_t lineNum() const;

protected:
    std::string nextLine();

protected:
    HeaderType header_;
    Extractor extractor_;

    std::string name_;
    InputStream& in_;
    uint64_t lineNum_;
    uint64_t valueCount_;

    bool cached_;
    bool cachedRv_;
    ValueType cachedValue_;
};

template<typename Extractor>
inline std::string const& TypedStream<Extractor>::name() const {
    return in_.name();
}

template<typename Extractor>
inline bool TypedStream<Extractor>::eof() const {
    if (cached_)
        return !cachedRv_;
    else
        return in_.eof();
}

template<typename Extractor>
inline bool TypedStream<Extractor>::peek(ValueType** value) {
    // already peeked and have a value to return
    if (cached_) {
        // we peeked but got EOF
        if (in_.eof())
            return false;

        *value = &cachedValue_;
        return true;
    }

    checkEof();

    // need to peek ahead
    // note: next() may return false (because of EOF). we want to take care
    // when using cachedValue_ to make sure that EOF is false.
    cachedRv_ = next(cachedValue_);
    *value = &cachedValue_;
    cached_ = true;
    return cachedRv_;
}

template<typename Extractor>
inline bool TypedStream<Extractor>::next(ValueType& value) {
    if (cached_) {
        value.swap(cachedValue_);
        cached_ = false;
        return cachedRv_;
    }

    std::string line = nextLine();
    if (line.empty())
        return false;

    try {
        extractor_(&header_, line, value);
    }
    catch (std::exception const& e) {
        using boost::format;
        throw std::runtime_error(
            str(format("Error at %1%:%2%: %3%"
                ) % name() % in_.lineNum() % e.what()));
    }
    ++valueCount_;
    return true;
}

template<typename Extractor>
inline std::string TypedStream<Extractor>::nextLine() {
    std::string line;
    do {
        in_.getline(line);
    } while (!eof() && (line.empty() || line[0] == '#'));
    return line;
}

template<typename Extractor>
inline uint64_t TypedStream<Extractor>::valueCount() const {
    return valueCount_;
}

template<typename Extractor>
inline void TypedStream<Extractor>::checkEof() const {
    if (!cached_ && eof())
        throw std::runtime_error("Attempted to read past eof of stream " + name());
}

template<typename Extractor>
inline uint64_t TypedStream<Extractor>::lineNum() const {
    return in_.lineNum();
}


template<typename ValueType_>
struct DefaultParser {
    typedef ValueType_ ValueType;
    typedef typename ValueType::HeaderType HeaderType;

    void operator()(HeaderType const* h, std::string& line, ValueType& entry) {
        ValueType::parseLine(h, line, entry);
    }
};

template<typename Parser>
struct TypedStreamFactory {
    typedef typename Parser::ValueType ValueType;
    typedef TypedStream<Parser> StreamType;

    TypedStreamFactory(Parser parser = Parser())
        : parser(parser)
    {}

    typename StreamType::ptr operator()(InputStream& in) {
        return std::make_unique<StreamType>(parser, in);
    }

    Parser parser;
};
