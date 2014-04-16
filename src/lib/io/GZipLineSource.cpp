#include "GZipLineSource.hpp"

#include <boost/format.hpp>

#include <cstddef>
#include <cstdio>

using boost::format;

namespace {
    static int const bufsz = 4096;
}

class GZipLineSource::LineBuffer {
public:
    enum Status {
        WHOLE_LINE = 0,
        PARTIAL_LINE
    };

    typedef char value_type;
    typedef size_t size_type;

    LineBuffer(size_type capacity)
        : _buf(capacity)
        , _beg(0u)
        , _end(0u)
    {
    }

    bool empty() const {
        return _end == 0;
    }

    void setEnd(size_type newEnd) {
        _end = newEnd;
    }

    value_type* buffer() {
        assert(_beg == 0);
        assert(_end == 0);
        return _buf.data();
    }

    value_type peek() const {
        assert(!empty());
        return _buf[_beg];
    }

    size_type size() const {
        return _buf.size();
    }

    template<typename Appendable>
    Status appendUntil(Appendable& tgt, value_type ch) {
        Status rv(PARTIAL_LINE);
        value_type const* first = _buf.data() + _beg;
        value_type const* last = _buf.data() + _end;
        value_type const* chPos = std::find(first, last, ch);
        size_t sz = chPos - first;
        tgt.append(first, sz);
        if (chPos == last) {
            _beg = _end = 0u;
        }
        else {
            _beg = chPos - _buf.data() + 1;
            rv = WHOLE_LINE;
        }

        if (_beg == _end) {
            _beg = _end = 0u;
        }

        return rv;
    }

private:
    std::vector<value_type> _buf;
    size_type _beg;
    size_type _end;
};



GZipLineSource::GZipLineSource(int fd)
    : _path(str(format("fd%1%") % fd))
    , _fp(gzdopen(fd, "rb"))
    , _buffer(new LineBuffer(bufferSize()))
    , _bad(_fp == Z_NULL)
    , _eof(false)
{
}

GZipLineSource::GZipLineSource(std::string const& path)
    : _path(path)
    , _fp(gzopen(path.c_str(), "rb"))
    , _buffer(new LineBuffer(bufsz))
    , _bad(_fp == Z_NULL)
    , _eof(false)
{
}

GZipLineSource::~GZipLineSource() {
    delete _buffer;
    gzclose(_fp);
}

bool GZipLineSource::getline(std::string& line) {
    line.erase();
    LineBuffer::Status lineStatus(LineBuffer::PARTIAL_LINE);

    if (!_buffer->empty()) {
        lineStatus = _buffer->appendUntil(line, '\n');
        if (lineStatus == LineBuffer::WHOLE_LINE) {
            return true;
        }
    }

    LineBuffer::value_type* data = _buffer->buffer();
    int sz;
    while ((sz = gzread(_fp, data, _buffer->size())) > 0) {
        _buffer->setEnd(sz);
        lineStatus = _buffer->appendUntil(line, '\n');
        if (lineStatus == LineBuffer::WHOLE_LINE) {
            break;
        }
    }

    _eof = !(lineStatus == LineBuffer::WHOLE_LINE || (line.size()));
    return !_eof;
}

char GZipLineSource::peek() {
    if (!_buffer->empty()) {
        return _buffer->peek();
    }

    int c = gzgetc(_fp);
    if (c > 0) {
        gzungetc(c, _fp);
    }
    return c;
}

bool GZipLineSource::eof() const {
    return _buffer->empty() && _eof;
}

bool GZipLineSource::good() const {
    return !_bad && !eof();
}

GZipLineSource::operator bool() const {
    return good();
}

size_t GZipLineSource::bufferSize() {
    return bufsz;
}
