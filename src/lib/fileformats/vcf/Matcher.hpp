#pragma once

#include "CustomValue.hpp"
#include "namespace.hpp"
#include <iostream>

#include <string>
#include <functional>

VCF_NAMESPACE_BEGIN

class Entry;

namespace Conditions {

    class Base {
    public:
        virtual ~Base() {}
        virtual bool operator()(const Entry& e) const = 0;
    };

    class Not {
    public:
        explicit Not(const Base& cond)
            : _cond(cond)
        {}

        bool operator()(const Entry& e) const {
            return !_cond(e);
        }

    protected:
        const Base& _cond;
    };


    template<typename T, typename Compare>
    class RelOp : public Base {
    public:
        typedef std::function<const T&(const Entry&)> Extractor;
        RelOp(const T& value, const Extractor& extractor, Compare cmp = Compare())
            : _value(value)
            , _extractor(extractor)
            , _cmp(cmp)
        {}

        bool operator()(const Entry& e) const {
            const T& value = _extractor(e);
            return _cmp(value, _value);
        }

    protected:
        const T& _value;
        Extractor _extractor;
        Compare _cmp;
    };

    template<typename Op>
    class BinaryLogical : public Base {
    public:
        BinaryLogical(const Base* a, const Base* b, Op op = Op())
            : _op(op)
            , _a(a)
            , _b(b)
        {}

        bool operator()(const Entry& e) const {
            return _op((*_a)(e), (*_b)(e));
        }

    protected:
        Op _op;
        const Base* _a;
        const Base* _b;
    };
}

VCF_NAMESPACE_END
