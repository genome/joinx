#include "common/Timer.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/StreamHandler.hpp"
#include "fileformats/VcfReader.hpp"

#include <iostream>

size_t testSharedPtr(VcfReader::ptr const& reader, std::ostream& out) {
    Vcf::Entry entry;
    size_t count(0);
    while (reader->next(entry)) {
        out << entry << "\n";
        ++count;
    }
    return count;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_vcf_file> <output_vcf_file>\n";
        return 1;
    }

    StreamHandler streams;
    InputStream::ptr inStream = streams.wrap<std::istream, InputStream>(argv[1]);
    ostream* out = streams.get<ostream>(argv[2]);
    auto reader= openVcf(*inStream);

    WallTimer timer;
    size_t count = testSharedPtr(reader, *out);
    std::cout << "Shared ptr: " << count << " entries in " << timer.elapsed() << "\n";

    return 0;
}
