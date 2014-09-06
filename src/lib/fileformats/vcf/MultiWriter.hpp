#pragma once

#include "common/namespaces.hpp"
#include "io/StreamHandler.hpp"

#include "Entry.hpp"
#include "Header.hpp"

#include <vector>
#include <string>

BEGIN_NAMESPACE(Vcf)

class MultiWriter {
public:
    MultiWriter(std::vector<std::string> const& filenames);

    void write(Entry const& e);

private:
    std::vector<std::string> const& filenames_;
    std::vector<bool> wroteHeader_;
    StreamHandler streams_;
};

END_NAMESPACE(Vcf)
