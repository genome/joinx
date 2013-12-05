#pragma once

#include <ostream>
#include <sstream>
#include <string>

template<typename Iterable>
struct StreamJoin {

    explicit StreamJoin(Iterable const& seq)
        : seq(seq)
        , empty(".")
        , delim(",")
    {
    }

    StreamJoin& emptyString(char const* empty) {
        this->empty = empty;
        return *this;
    }

    StreamJoin& delimiter(char const* delim) {
        this->delim = delim;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& out, StreamJoin const& sj) {
        if (sj.seq.empty()) {
            out << sj.empty;
        }
        else {
            auto i = sj.seq.begin();
            out << *i;
            for (++i; i != sj.seq.end(); ++i) {
                out << sj.delim << *i;
            }
        }
        return out;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    Iterable const& seq;
    char const* empty;
    char const* delim;
};


template<typename Iterable>
StreamJoin<Iterable> streamJoin(Iterable const& seq) {
    return StreamJoin<Iterable>(seq);
}
