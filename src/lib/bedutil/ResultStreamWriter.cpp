#include "ResultStreamWriter.hpp"

#include "fileformats/Bed.hpp"

using namespace std;

ResultStreamWriter::ResultStreamWriter(
        std::ostream* hitA,
        std::ostream* hitB,
        std::ostream* missA,
        std::ostream* missB)
    : _hitA(hitA)
    , _hitB(hitB)
    , _missA(missA)
    , _missB(missB)
{
}

void ResultStreamWriter::hitA(const Bed& a) {
    if (_hitA)
        *_hitA << a.line << endl;
}

void ResultStreamWriter::hitB(const Bed& b) {
    if (_hitB)
        *_hitB << b.line << endl;
}

void ResultStreamWriter::missA(const Bed& a) {
    if (_missA)
        *_missA << a.line << endl;
}

void ResultStreamWriter::missB(const Bed& b) {
    if (_missB)
        *_missB << b.line << endl;
}
