#include "MutationSpectrum.hpp" 

#include <boost/format.hpp>
#include <limits>
#include <stdexcept>

using boost::format;

MutationSpectrum::MutationSpectrum() {
    _indexTable.fill(-1);
    _mtx.fill(0);
    _indexTable['A'] = 0;
    _indexTable['C'] = 1;
    _indexTable['G'] = 2;
    _indexTable['T'] = 3;
    _indexTable['a'] = 0;
    _indexTable['c'] = 1;
    _indexTable['g'] = 2;
    _indexTable['t'] = 3;
}

uint64_t& MutationSpectrum::operator()(char from, char to) {
    return _mtx[index(from, to)];
}

uint64_t const& MutationSpectrum::operator()(char from, char to) const{
    return _mtx[index(from, to)];
}

uint64_t MutationSpectrum::transitions() const {
    return (*this)('A', 'G') + (*this)('C', 'T');
}

uint64_t MutationSpectrum::transversions() const {
    return (*this)('A', 'C') + (*this)('A', 'T')
        + (*this)('C', 'A') + (*this)('C', 'G');
}

// returns numeric_limits<double>::infinity if transversions=0
double MutationSpectrum::transitionTransversionRatio() const {
    uint64_t tv(transversions());
    if (tv == 0)
        return std::numeric_limits<double>::infinity();
    uint64_t ts(transitions());
    return ts/double(tv);
}

int MutationSpectrum::index(char from, char to) const {
    int fromIdx = _indexTable[from];
    int toIdx = _indexTable[to];
    if (fromIdx < 0 || toIdx < 0)
        throw std::runtime_error(str(format("Invalid alleles for mutation spectrum: %1%->%2%") %from %to));
    return fromIdx*4+toIdx;
}
