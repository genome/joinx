#include "MutationSpectrum.hpp"

#include <boost/format.hpp>
#include <limits>
#include <stdexcept>

using boost::format;

namespace {
    const static std::string bases = "ACGT";
    int const _indexTable[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1,  0, -1,  1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1,  0, -1,  1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };
}

MutationSpectrum::MutationSpectrum() {
    _mtx.fill(0);
}

uint64_t& MutationSpectrum::operator()(uint8_t from, uint8_t to) {
    return _mtx[index(from, to)];
}

uint64_t const& MutationSpectrum::operator()(uint8_t from, uint8_t to) const{
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

int MutationSpectrum::index(uint8_t from, uint8_t to) const {
    int fromIdx = _indexTable[int(from)];
    int toIdx = _indexTable[int(to)];
    if (fromIdx < 0 || toIdx < 0)
        throw InvalidAlleleError(str(format("Invalid alleles for mutation spectrum: %1%->%2%") %from %to));
    return fromIdx*4+toIdx;
}

std::ostream& operator<<(std::ostream& s, MutationSpectrum const& ms) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int n = ms(bases[i], bases[j]);
            if (n) {
                s << bases[i] << "->" << bases[j] << ": " << n << "\n";
            }
        }
    }
    return s;
}
