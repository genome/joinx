#pragma once

template<typename Iter>
class CyclicIterator {
public:
    typedef typename Iter::value_type const value_type;

    CyclicIterator(Iter const& begin, Iter const& end)
        : _begin(begin)
        , _end(end)
        , _pos(begin)
    {
    }

    value_type& operator*() {
        return *_pos;
    }

    value_type const& operator*() const {
        return *_pos;
    }

    CyclicIterator operator++(int) {
        CyclicIterator rv(*this);
        if (++_pos == _end)
            _pos = _begin;
        return rv;
    }

    CyclicIterator& operator++() {
        if (++_pos == _end)
            _pos = _begin;

        return *this;
    }

    bool operator==(CyclicIterator const& rhs) const {
        return _pos == rhs._pos;
    }

    bool operator!=(CyclicIterator const& rhs) const {
        return !(*this == rhs);
    }

protected:
    Iter const& _begin;
    Iter const& _end;
    Iter _pos;
};
