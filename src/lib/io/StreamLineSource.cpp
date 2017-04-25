#include "StreamLineSource.hpp"

StreamLineSource::StreamLineSource(std::istream& in)
    : _in(in)
{
}

bool StreamLineSource::getline(std::string& line) {
    return bool(std::getline(_in, line));
}

char StreamLineSource::peek() {
    return _in.peek();
}

bool StreamLineSource::eof() const {
    return _in.eof();
}

bool StreamLineSource::good() const {
    return _in.good();
}

StreamLineSource::operator bool() const {
    return !_in.fail();
}
