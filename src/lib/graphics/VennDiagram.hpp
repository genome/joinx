#pragma once

#include "Primitives.hpp"

#include <boost/unordered_map.hpp>

#include <cstddef>
#include <string>
#include <vector>

struct VennLayouts {
    struct VennLayout {
        std::vector<FilledCircle> circles;
        std::vector<std::pair<Point, Text::Align>> textPos;
        double radius;
        double fillAlpha;
        double fontSize;
        double tpad;
    };

    VennLayouts();

    VennLayout const& get(size_t n) const;

    boost::unordered_map<size_t, VennLayout> geometries_;
};

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
    VennLayouts geometries_;
};
