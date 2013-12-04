#include "VennDiagram.hpp"
#include "Legend.hpp"
#include "Primitives.hpp"

#include <cairomm/context.h>
#include <cairomm/surface.h>

#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

using boost::format;
using boost::assign::list_of;
using boost::lexical_cast;


VennLayouts::VennLayouts() {
    VennLayout three;
    three.radius = 2.5;
    three.fillAlpha = 0.3;
    three.fontSize = 0.3;
    three.tpad = 0.08;

    three.circles.push_back
        (FilledCircle(Point(4, 4), three.radius, Color(1, 0, 0), Color(1, 0, 0, three.fillAlpha)));
    three.circles.push_back
        (FilledCircle(Point(6, 4), three.radius, Color(0, 1, 0), Color(0, 1, 0, three.fillAlpha)));
    three.circles.push_back
        (FilledCircle(Point(5, 6), three.radius, Color(0, 0, 1), Color(0, 0, 1, three.fillAlpha)));

    three.textPos.push_back
        (std::make_pair(Point(4.0 - three.radius + three.tpad, 4.0), Text::AlignLeft));
    three.textPos.push_back
        (std::make_pair(Point(6.0 + three.radius - three.tpad, 4.0), Text::AlignRight));
    three.textPos.push_back
        (std::make_pair(Point(5.0, 3.0), Text::AlignCenter));
    three.textPos.push_back
        (std::make_pair(Point(5.0, 6.0 + three.radius / 2.0), Text::AlignCenter));
    three.textPos.push_back
        (std::make_pair(Point(2.666, 6.0), Text::AlignLeft));
    three.textPos.push_back
        (std::make_pair(Point(7.333, 6.0), Text::AlignRight));
    three.textPos.push_back
        (std::make_pair(Point(5.0, 5.0), Text::AlignCenter));


    VennLayout two;
    two.radius = 3.2;
    two.fillAlpha = 0.3;
    two.fontSize = 0.3;
    two.tpad = 0.10;

    two.circles.push_back
        (FilledCircle(Point(3.5, 5), two.radius, Color(1, 0, 0), Color(1, 0, 0, two.fillAlpha)));
    two.circles.push_back
        (FilledCircle(Point(6.5, 5), two.radius, Color(0, 0, 1), Color(0, 0, 1, two.fillAlpha)));

    two.textPos.push_back
        (std::make_pair(Point(3.5 - two.radius + two.tpad, 5.0), Text::AlignLeft));
    two.textPos.push_back
        (std::make_pair(Point(6.5 + two.radius - two.tpad, 5.0), Text::AlignRight));
    two.textPos.push_back
        (std::make_pair(Point(5.0, 5.0), Text::AlignCenter));

    geometries_[3] = three;
    geometries_[2] = two;
}

auto VennLayouts::get(size_t n) const -> VennLayout const& {
    auto iter = geometries_.find(n);
    if (iter == geometries_.end()) {
        throw std::runtime_error(str(format(
            "Unsupported number of sets (%1%) for venn diagram"
            ) % n));
    }
    return iter->second;
}

VennDiagram::VennDiagram(
        std::vector<std::string> const& setNames,
        std::vector<size_t> const& counts
        )
    : setNames_(setNames)
    , counts_(counts)
{
    geometries_.get(setNames_.size());

    size_t expectedNumCounts = (1 << setNames_.size()) - 1;
    if (counts_.size() != expectedNumCounts) {
        throw std::runtime_error(str(format(
            "Wrong number of counts for %1% sets (given %2%, expected %3%)"
            ) % setNames_.size() % counts_.size() % expectedNumCounts));
    }
}

VennDiagram& VennDiagram::setTitle(std::string title) {
    title_.swap(title);
    return *this;
}

void VennDiagram::draw(std::string const& filename, int width, int height) {
    Cairo::RefPtr<Cairo::PdfSurface> surface(
            Cairo::PdfSurface::create(filename, width, height)
            );

    Cairo::RefPtr<Cairo::Context> ctx(
            Cairo::Context::create(surface)
            );

    // set scale so that (0, 0) is upper left, (10, 10) is lower right;
    ctx->scale(width / 10.0, height / 10.0);
    ctx->save();
    ctx->set_source_rgb(1.0, 1.0, 1.0);
    ctx->paint();
    ctx->restore(); // restore color to black

    ctx->set_line_width(0.05);

    auto geom = geometries_.get(setNames_.size());

    ctx->save();
    ctx->set_line_width(0.01);
    for (auto i = geom.circles.begin(); i != geom.circles.end(); ++i) {
        i->render(ctx);
    }

    ctx->restore();

    Font font("Helvetica", geom.fontSize);
    for (size_t i = 0; i < counts_.size(); ++i) {
        auto x = geom.textPos[i];
        auto point = x.first;
        auto align = x.second;
        Text txt(point, counts_[i], font, align);
        txt.render(ctx);
    }

    Font titleFont("Helvetica", 0.4, Cairo::FONT_WEIGHT_BOLD);
    auto titleExtents = titleFont.text_extents(ctx, title_);
    double titleY = titleExtents.height * 3;
    Text(Point(5, titleY), title_, titleFont, Text::AlignCenter).render(ctx);

    std::vector<std::tuple<std::string, Color>> labels;
    for (size_t i = 0; i < setNames_.size(); ++i) {
        auto const& color = geom.circles[i].fillColor;
        labels.push_back(std::make_tuple(setNames_[i], color));
    }
    Legend legend(font, labels);
    legend.render(ctx);

    ctx->show_page();
}
