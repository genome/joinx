#pragma once

#include <cstddef>
#include <string>
#include <vector>

class VennDiagram {
public:
    VennDiagram(
        std::vector<std::string> const& setNames,
        std::vector<size_t> const& counts
        );

    VennDiagram& setTitle(std::string title);

    void draw(std::string const& filename, int width, int height);

private:
    std::string title_;
    std::vector<std::string> const& setNames_;
    std::vector<size_t> const& counts_;
};
