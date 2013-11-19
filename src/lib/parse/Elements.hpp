#pragma once

#include "common/Tokenizer.hpp"
#include "common/StringView.hpp"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/vector.hpp>

#include <tuple>
#include <cassert>
#include <functional>
#include <iterator>
#include <ostream>
#include <set>
#include <string>
#include <vector>

struct ImmutableTag {
    enum {MUTABLE = 0};
};

struct MutableTag {
    enum {MUTABLE = 1};
};

template<typename T>
struct BasicField {
    typedef T value_type;
    T value;

    // IterType is an iterator into a list of strings, e.g.,
    // vector<string>::const_iterator or
    // vector<StringView>::const_iterator
    template<typename IterType>
    bool parse(IterType begin, IterType end) {
        assert(end - begin == 1);
        Tokenizer<char> tok(*begin, '\0');
        return tok.extract(value) && tok.eof();
    }

    void toStream(std::ostream& s) const {
        s << value;
    }
};

template<typename T, char delim_>
struct DelimitedField {
    typedef T value_type;
    T value;

    template<typename IterType>
    bool parse(IterType begin, IterType end) {
        assert(end - begin == 1);
        Tokenizer<char>::split(*begin, delim_, std::back_inserter(value));
        return true;
    }

    void toStream(std::ostream& s) const {
        if (value.empty()) {
            s << ".";
        }
        else {
            bool first = true;
            for (auto iter = value.begin(); iter != value.end(); ++iter) {
                auto const& x = *iter;
                if (first) {
                    first = false;
                }
                else {
                    s << delim_;
                }
                s << x;
            }
        }
    }
};

template<size_t idx, typename FieldType>
struct Column : public FieldType {
    template<typename Iter>
    std::tuple<Iter, Iter> selectFields(Iter begin, Iter) {
        return std::make_tuple(begin + idx, begin + idx + 1);
    }
};


template<size_t idx1, typename FieldType>
struct RightColumns : public FieldType {
    template<typename Iter>
    std::tuple<Iter, Iter> selectFields(Iter begin, Iter end) {
        return std::make_tuple(begin + idx1, end);
    }
};

template<typename Field, typename T>
typename Field::value_type& getField(T& x) {
    return static_cast<Field&>(x).value;
}

template<typename Field, typename T>
typename Field::value_type const& getField(T const& x) {
    return static_cast<Field const&>(x).value;
}

struct TabDelimited {
    static char delimChar() {
        return '\t';
    }

    static char const* delimString() {
        return "\t";
    }
};

template<typename TypeList, typename Delim, typename Mutability = ImmutableTag>
class Record : public boost::mpl::inherit_linearly<
                    TypeList,
                    boost::mpl::inherit<boost::mpl::_1, boost::mpl::_2>::type>::type
{
private:
    struct ParseHelper_ {
        ParseHelper_(std::vector<StringView> const& vals, Record& record)
            : vals(vals)
            , record(record)
        {
        }

        template<typename Element>
        void operator()(Element&) const {
            auto& e = static_cast<Element&>(record);
            std::vector<StringView>::const_iterator beg, end;
            std::tie(beg, end) = e.selectFields(vals.begin(), vals.end());
            e.parse(beg, end);
        }

        std::vector<StringView> const& vals;
        Record& record;
    };

    struct PrintHelper_ {
        PrintHelper_(Record const& record, std::ostream& out)
            : record(record)
            , out(out)
            , first(true)
        {
        }

        template<typename Element>
        void operator()(Element const&) {
            if (first) {
                first = false;
            }
            else {
                out << Delim::delimChar();
            }
            static_cast<Element const&>(record).toStream(out);
        }

        Record const& record;
        std::ostream& out;
        bool first;
    };

public:
    template<typename FieldType>
    typename FieldType::value_type get() {
        return getField<FieldType>(*this);
    }

    template<typename FieldType>
    typename FieldType::value_type get() const {
        return getField<FieldType>(*this);
    }

    bool parse(std::string s) {
        line_.swap(s);

        std::vector<StringView> tokens;
        Tokenizer<char>::split(line_, '\t', std::back_inserter(tokens));

        ParseHelper_ helper(tokens, *this);
        boost::mpl::for_each<TypeList>(helper);
        return true;
    }

    void toStream(std::ostream& s) const {
        if (Mutability::MUTABLE == 1) {
            PrintHelper_ ph(*this, s);
            boost::mpl::for_each<TypeList>(ph);
        }
        else {
            s << line_;
        }
    }

private:
    std::string line_;
};
