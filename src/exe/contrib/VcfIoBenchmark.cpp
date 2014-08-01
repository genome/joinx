#include "common/Timer.hpp"
#include "io/InputStream.hpp"
#include "io/StreamHandler.hpp"
#include "fileformats/VcfReader.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct IBenchmark {
    virtual ~IBenchmark() {}

    virtual std::string name() const = 0;
    virtual size_t run(VcfReader::ptr const& reader, std::ostream& out) const = 0;
};

struct TestInOut : public IBenchmark {

    std::string name() const { return "input and output"; }
    size_t run(VcfReader::ptr const& reader, std::ostream& out) const {
        Vcf::Entry entry;
        size_t count(0);
        auto& r = *reader;
        while (r.next(entry)) {
            out << entry << "\n";
            ++count;
        }
        return count;
    }
};

struct TestInOnly : public IBenchmark {
    std::string name() const { return "input only"; }

    size_t run(VcfReader::ptr const& reader, std::ostream& out) const {
        Vcf::Entry entry;
        size_t count(0);
        auto& r = *reader;
        while (r.next(entry)) {
            entry.sampleData();
            ++count;
        }
        return count;

    }
};


int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_vcf_file> <output_vcf_file>\n";
        return 1;
    }

    boost::ptr_vector<IBenchmark> tests;
//    tests.push_back(new TestInOut);
    tests.push_back(new TestInOnly);

    for (auto iter = tests.begin(); iter != tests.end(); ++iter) {
        StreamHandler streams;
        auto in = streams.openForReading(argv[1]);
        auto reader = openVcf(*in);
        ostream* out = streams.get<ostream>(argv[2]);

        WallTimer timer;
        size_t count = iter->run(reader, *out);
        std::cout << iter->name() << ": " << count << " entries in "
            << timer.elapsed() << "\n";
    }

    return 0;
}
