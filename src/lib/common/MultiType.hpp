#pragma once

// We were using boost::any, but it was slower

class MultiType;

namespace {
    template<typename T>
    inline T const* multiGet(MultiType const* v);

    template<typename T>
    T& multiGet(MultiType& v);
}

class MultiType {
public:
    MultiType() : _empty(true) {}

    MultiType& operator=(std::string const& s) {
        _empty = false;
        _s = s;
        return *this;
    }
    MultiType& operator=(int64_t i) {
        _empty = false;
        _i = i;
        return *this;
    }
    MultiType& operator=(double f) {
        _empty = false;
        _f = f;
        return *this;
    }
    MultiType& operator=(char c) {
        _empty = false;
        _c = c;
        return *this;
    }

    MultiType& operator=(bool b) {
        _empty = false;
        _b = b;
        return *this;
    }

    template<typename T>
    T const* get() const {
        return multiGet<T>(this);
    }

    template<typename T>
    T& ref() {
        _empty = false;
        return multiGet<T>(*this);
    }

    bool empty() const { return _empty; }

    std::string _s;
    int64_t _i;
    double _f;
    char _c;
    bool _b; 

protected:
    bool _empty;
};

namespace {
    template<typename T>
    inline T const* multiGet(MultiType const* v) {
        throw std::runtime_error("multiGet called with unsupported type!");
        return 0;
    }

    template<>
    inline std::string const* multiGet<std::string>(MultiType const* v) {
        if (v->empty()) return 0;
        return &v->_s;
    }

    template<>
    inline int64_t const* multiGet<int64_t>(MultiType const* v) {
        if (v->empty()) return 0;
        return &v->_i;
    }

    template<>
    inline double const* multiGet<double>(MultiType const* v) {
        if (v->empty()) return 0;
        return &v->_f;
    }

    template<>
    inline char const* multiGet<char>(MultiType const* v) {
        if (v->empty()) return 0;
        return &v->_c;
    }

    template<>
    inline bool const* multiGet<bool>(MultiType const* v) {
        if (v->empty()) return 0;
        return &v->_b;
    }

    template<typename T>
    T& multiGet(MultiType& v) {
        throw std::runtime_error("multiGet called with unsupported type!");
    }

    template<>
    inline std::string& multiGet<std::string>(MultiType& v) {
        return v._s;
    }

    template<>
    inline int64_t& multiGet<int64_t>(MultiType& v) {
        return v._i;
    }

    template<>
    inline double& multiGet<double>(MultiType& v) {
        return v._f;
    }

    template<>
    inline char& multiGet<char>(MultiType& v) {
        return v._c;
    }

    template<>
    inline bool& multiGet<bool>(MultiType& v) {
        return v._b;
    }
}
