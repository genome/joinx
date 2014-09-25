#pragma once

#include "io/InputStream.hpp"

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

//TODO Needs accessor for lineNum
template<typename ValueClass>
class TypedStreamFilterBase {
public:
    typedef ValueClass ValueType;

    TypedStreamFilterBase() : filtered_(0) {}
    virtual ~TypedStreamFilterBase() {}

    uint64_t filtered() {
        return filtered_;
    }

    bool exclude(const ValueType& snv) {
        if (exclude_(snv)) {
            ++filtered_;
            return true;
        }
        return false;
    }

protected:
    virtual bool exclude_(const ValueType& snv) = 0;

protected:
    uint64_t filtered_;
};

template<typename ValueClass, typename Extractor>
class TypedStream {
public:
    typedef typename ValueClass::HeaderType HeaderType;
    typedef ValueClass ValueType;
    typedef TypedStreamFilterBase<ValueType> FilterType;
    typedef boost::shared_ptr<TypedStream<ValueClass, Extractor> > ptr;

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

    void addFilter(FilterType* filter);

    std::string const& name() const;
    bool eof() const;
    bool peek(ValueType** value);
    bool exclude(const ValueType& value) const;
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

    std::vector<FilterType*> filters_;
};

template<typename ValueType, typename Extractor>
inline std::string const& TypedStream<ValueType, Extractor>::name() const {
    return in_.name();
}

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::eof() const {
    if (cached_)
        return !cachedRv_;
    else
        return in_.eof();
}

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::peek(ValueType** value) {
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

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::exclude(const ValueType& value) const {
    for (auto iter = filters_.begin(); iter != filters_.end(); ++iter) {
        if ((*iter)->exclude(value))
            return true;
    }

    return false;
}

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::next(ValueType& value) {
    if (cached_) {
        value.swap(cachedValue_);
        cached_ = false;
        return cachedRv_;
    }

    do {
        std::string line = nextLine();
        if (line.empty())
            return false;

        try {
            extractor_(&header_, line, value);
        }
        catch (const std::exception& e) {
            using boost::format;
            throw std::runtime_error(
                str(format("Error at %1%:%2%: %3%"
                    ) % name() % in_.lineNum() % e.what()));
        }
    } while (exclude(value));
    ++valueCount_;
    return true;
}

template<typename ValueType, typename Extractor>
inline std::string TypedStream<ValueType, Extractor>::nextLine() {
    std::string line;
    do {
        in_.getline(line);
    } while (!eof() && (line.empty() || line[0] == '#'));
    return line;
}

template<typename ValueType, typename Extractor>
inline uint64_t TypedStream<ValueType, Extractor>::valueCount() const {
    return valueCount_;
}

template<typename ValueType, typename Extractor>
inline void TypedStream<ValueType, Extractor>::checkEof() const {
    if (!cached_ && eof())
        throw std::runtime_error("Attempted to read past eof of stream " + name());
}

template<typename ValueType, typename Extractor>
inline uint64_t TypedStream<ValueType, Extractor>::lineNum() const {
    return in_.lineNum();
}

template<typename ValueType, typename Extractor>
inline void TypedStream<ValueType, Extractor>::addFilter(FilterType* filter) {
    filters_.push_back(filter);
}

