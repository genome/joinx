#include "MultiWriter.hpp"

#include <boost/format.hpp>

#include <stdexcept>

using boost::format;

BEGIN_NAMESPACE(Vcf)

MultiWriter::MultiWriter(std::vector<std::string> const& filenames)
    : filenames_(filenames)
    , wroteHeader_(filenames.size(), false)
{
}

void MultiWriter::write(Entry const& e) {
    size_t index = e.header().sourceIndex();
    if (index >= filenames_.size()) {
        throw std::runtime_error(str(format(
            "MultiWriter configured with %1% output streams, but entry:\n"
            "%2%\nhas source index %3%"
            ) % filenames_.size() % e % index));
    }

    std::ostream* out = streams_.get<std::ostream>(filenames_[index]);
    if (!wroteHeader_[index]) {
        *out << e.header();
        wroteHeader_[index] = true;
    }
    *out << e << "\n";
}

END_NAMESPACE(Vcf)
