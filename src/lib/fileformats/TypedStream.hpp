#pragma once

#include "InputStream.hpp"

#include <boost/format.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

template<typename ValueClass>
class TypedStreamFilterBase {
public:
    typedef ValueClass ValueType;

    TypedStreamFilterBase() : _filtered(0) {}
    ~TypedStreamFilterBase() {}

    uint64_t filtered() {
        return _filtered;
    }

    bool exclude(const ValueType& snv) {
        if (_exclude(snv)) {
            ++_filtered;
            return true;
        }
        return false;
    }

protected:
    virtual bool _exclude(const ValueType& snv) = 0;

protected:
    uint64_t _filtered;
};

template<typename ValueClass, typename Extractor>
class TypedStream {
public:
    typedef ValueClass ValueType;
    typedef TypedStreamFilterBase<ValueType> FilterType;
    typedef std::shared_ptr<TypedStream<ValueClass, Extractor> > ptr;

    TypedStream(Extractor& extractor, InputStream& in)
        : _extractor(extractor)
        , _in(in)
        , _lineNum(0)
        , _valueCount(0)
        , _cached(false)
        , _cachedRv(false)
    {
    }

    void addFilter(FilterType* filter);

    const std::string& name() const;
    bool eof() const;
    bool peek(ValueType** value);
    bool exclude(const ValueType& value) const;
    bool next(ValueType& value);
    uint64_t valueCount() const;
    void checkEof() const;

protected:
    std::string nextLine();

protected:
    Extractor _extractor;

    std::string _name;
    InputStream& _in;
    int _maxExtraFields;
    uint64_t _lineNum;
    uint64_t _valueCount;

    bool _cached;
    bool _cachedRv;
    ValueType _cachedValue;

    std::vector<FilterType*> _filters;
};

template<typename ValueType, typename Extractor>
inline const std::string& TypedStream<ValueType, Extractor>::name() const {
    return _in.name();
}

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::eof() const {
    if (_cached)
        return !_cachedRv;
    else
        return _in.eof();
}

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::peek(ValueType** value) {
    // already peeked and have a value to return
    if (_cached) {
        // we peeked but got EOF
        if (_in.eof())
            return false;

        *value = &_cachedValue;
        return true;
    }

    checkEof();

    // need to peek ahead
    // note: next() may return false (because of EOF). we want to take care
    // when using _cachedValue to make sure that EOF is false.
    _cachedRv = next(_cachedValue);
    *value = &_cachedValue;
    _cached = true;
    return _cachedRv;
}

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::exclude(const ValueType& value) const {
    for (auto iter = _filters.begin(); iter != _filters.end(); ++iter) {
        if ((*iter)->exclude(value))
            return true;
    }
        
    return false;
}

template<typename ValueType, typename Extractor>
inline bool TypedStream<ValueType, Extractor>::next(ValueType& value) {
    if (_cached) {
        value.swap(_cachedValue);
        _cached = false;
        return _cachedRv;
    }

    do {
        std::string line = nextLine();
        if (line.empty())
            return false;

        try {
            _extractor(line, value);
        } catch (const std::exception& e) {
            using boost::format;
            throw std::runtime_error(
                str(format("Error at %1%:%2%: %3%") %name() %_in.lineNum() %e.what()));
        }
    } while (exclude(value));
    ++_valueCount;
    return true;
}

template<typename ValueType, typename Extractor>
inline std::string TypedStream<ValueType, Extractor>::nextLine() {
    std::string line;
    do {
        _in.getline(line);
        ++_lineNum;
    } while (!eof() && (line.empty() || line[0] == '#'));
    return line;
}

template<typename ValueType, typename Extractor>
inline uint64_t TypedStream<ValueType, Extractor>::valueCount() const {
    return _valueCount;
}

template<typename ValueType, typename Extractor>
inline void TypedStream<ValueType, Extractor>::checkEof() const {
    if (!_cached && eof())
        throw std::runtime_error("Attempted to read past eof of stream " + name());
}

template<typename ValueType, typename Extractor>
inline void TypedStream<ValueType, Extractor>::addFilter(FilterType* filter) {
    _filters.push_back(filter);
}

