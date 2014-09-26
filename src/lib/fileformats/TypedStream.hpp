#pragma once

#include "common/compat.hpp"
#include "io/InputStream.hpp"

#include <boost/format.hpp>

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


template<typename Parser>
class TypedStream {
public:
    typedef typename Parser::ValueType ValueType;
    typedef typename ValueType::HeaderType HeaderType;
    typedef std::unique_ptr<TypedStream<Parser>> ptr;

    TypedStream(Parser& extractor, InputStream& in)
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
    Parser extractor_;

    std::string name_;
    InputStream& in_;
    uint64_t lineNum_;
    uint64_t valueCount_;

    bool cached_;
    bool cachedRv_;
    ValueType cachedValue_;
};

template<typename Parser>
inline std::string const& TypedStream<Parser>::name() const {
    return in_.name();
}

template<typename Parser>
inline bool TypedStream<Parser>::eof() const {
    if (cached_)
        return !cachedRv_;
    else
        return in_.eof();
}

template<typename Parser>
inline bool TypedStream<Parser>::peek(ValueType** value) {
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

template<typename Parser>
inline bool TypedStream<Parser>::next(ValueType& value) {
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

template<typename Parser>
inline std::string TypedStream<Parser>::nextLine() {
    std::string line;
    do {
        in_.getline(line);
    } while (!eof() && (line.empty() || line[0] == '#'));
    return line;
}

template<typename Parser>
inline uint64_t TypedStream<Parser>::valueCount() const {
    return valueCount_;
}

template<typename Parser>
inline void TypedStream<Parser>::checkEof() const {
    if (!cached_ && eof())
        throw std::runtime_error("Attempted to read past eof of stream " + name());
}

template<typename Parser>
inline uint64_t TypedStream<Parser>::lineNum() const {
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
    typedef typename StreamType::ptr StreamPtr;

    template<typename... Args>
    TypedStreamFactory(Args&&... args)
        : parser(std::forward<Args>(args)...)
    {}

    StreamPtr operator()(InputStream& in) {
        return std::make_unique<StreamType>(parser, in);
    }

    StreamPtr operator()(InputStream::ptr const& in) {
        return std::make_unique<StreamType>(parser, *in);
    }

    std::vector<StreamPtr> operator()(std::vector<InputStream::ptr>& ins) {
        std::vector<StreamPtr> rv;
        rv.reserve(ins.size());
        for (auto i = ins.begin(); i != ins.end(); ++i) {
            rv.push_back((*this)(*i));
        }
        // old gcc strikes again
        //std::transform(ins.begin(), ins.end(), std::back_inserter(rv), std::ref(*this));
        return rv;
    }

    Parser parser;
};
