#pragma once

#include "common/compat.hpp"
#include "common/namespaces.hpp"

#include <memory>
#include <ostream>
#include <string>
#include <utility>

BEGIN_NAMESPACE(Vcf)

class Header;

// Store a string representation of an object for optional parsing later.
//
// For type T to be used with LazyValue<T>, it must have a constructor of
// the form:
//   T(Vcf::Header const&, std::string const& text).
//
// as well as a copy constructor.
//
// It must also overload operator << for output to std::ostream
//
// To force parsing and get the resulting object:
//
// LazyValue<T> lazy(...);
// T const& t = lazy.get(header);
//
// Additional arguments for T's constructor can be passed to get as well:
//
// T const& t = lazy.get(header, foo, bar, baz, ...);
template<typename T>
class LazyValue {
public:
    LazyValue() {}
    LazyValue(std::string text)
        : text_(std::move(text))
    {}

    LazyValue(LazyValue const& other)
        : text_(other.text_)
    {
        data_.reset();
        if (other.data_)
            data_ = std::make_unique<T>(*other.data_);
    }


    LazyValue& operator=(LazyValue&& other) {
        text_ = std::move(other.text_);
        data_ = std::move(other.data_);
        return *this;
    }

    LazyValue& operator=(LazyValue const& other) {
        text_ = other.text_;
        data_.reset();
        if (other.data_)
            data_ = std::make_unique<T>(*other.data_);
        return *this;
    }

    LazyValue(LazyValue&& other)
        : text_(std::move(other.text_))
        , data_(std::move(other.data_))
    {
    }

    template<typename... Args>
    T& get(Header const& header, Args&&... args) {
        parse(header, std::forward<Args>(args)...);
        return *data_;
    }

    template<typename... Args>
    T const& get(Header const& header, Args&&... args) const {
        parse(header, std::forward<Args>(args)...);
        return *data_;
    }

    void clear() {
        text_.clear();
        data_.reset();
    }

    friend std::ostream& operator<<(std::ostream& os, LazyValue const& x) {
        if (x.data_) {
            os << *x.data_;
        }
        else if (!x.text_.empty()) {
            os << x.text_;
        }
        else {
            os << ".";
        }
        return os;
    }

    void swap(LazyValue& rhs) {
        text_.swap(rhs.text_);
        data_.swap(rhs.data_);
    }

private:
    template<typename... Args>
    void parse(Header const& header, Args&&... args) const {
        if (data_)
            return;

        data_ = std::make_unique<T>(header, text_, std::forward<Args>(args)...);
        text_.clear();
    }

private:
    mutable std::string text_;
    mutable std::unique_ptr<T> data_;
};

END_NAMESPACE(Vcf)
