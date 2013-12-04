#pragma once

#include "Primitives.hpp"

#include <vector>
#include <utility>
#include <string>
#include <tuple>

struct Legend : IRenderElement {
    Legend(
            Font font,
            std::vector<std::tuple<std::string, Color>> const& labels)
        : font(font)
        , labels(labels)
    {
    }

    void render(Cairo::RefPtr<Cairo::Context>& ctx) {
        ctx->save();
        Color(0,0,0).render(ctx);
        font.render(ctx);
        ctx->set_line_width(0.02);

        std::vector<Cairo::TextExtents> extents(labels.size());

        double maxWidth(0.0);
        double maxHeight(0.0);
        double totalHeight(0.0);

        for (size_t i = 0; i < labels.size(); ++i) {
            ctx->get_text_extents(std::get<0>(labels[i]), extents[i]);
            maxWidth = std::max(maxWidth, extents[i].width);
            maxHeight = std::max(maxHeight, extents[i].height);
            totalHeight += extents[i].height * 1.1;
        }

        double boxDim = maxHeight * 0.6;

        double y = extents[0].height * 1.1;
        double x = 0.1;
        ctx->rectangle(0, 0, x * 2 + boxDim * 1.1 + maxWidth * 1.1, totalHeight * 1.1 + y);
        ctx->stroke();

        ctx->move_to(0, y);
        for (size_t i = 0; i < labels.size(); ++i) {
            ctx->save();
            auto fillColor = std::get<1>(labels[i]);
            auto strokeColor = fillColor;
            strokeColor.a = 1.0;
            ctx->rectangle(x, y - extents[i].height / 2.0 - 0.05, boxDim, boxDim);
            strokeColor.render(ctx);
            ctx->stroke_preserve();
            fillColor.render(ctx);
            ctx->fill();
            ctx->restore();

            Text txt(Point(x + boxDim * 1.2, y), std::get<0>(labels[i]), font, Text::AlignLeft);
            txt.render(ctx);
            y += extents[i].height * 1.2;
        }

        ctx->restore();
    }

    Font font;
    std::vector<std::tuple<std::string, Color>> labels;
};
