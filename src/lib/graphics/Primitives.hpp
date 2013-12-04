#pragma once

#include <cairomm/context.h>

#include <boost/lexical_cast.hpp>

#include <string>
#include <utility>

class IRenderElement {
public:
    virtual ~IRenderElement() {}

    virtual void render(Cairo::RefPtr<Cairo::Context>& ctx) = 0;
};

struct Color : IRenderElement {
    Color() : r(0.0), g(0.0), b(0.0), a(1.0) {}
    Color(double r, double g, double b) : r(r), g(g), b(b), a(1.0) {}
    Color(double r, double g, double b, double a) : r(r), g(g), b(b), a(a) {}

    double r, g, b, a;

    void render(Cairo::RefPtr<Cairo::Context>& ctx) {
        ctx->set_source_rgba(r, g, b, a);
    }
};

struct Point {
    Point() : x(0.0), y(0.0) {}
    Point(double x, double y) : x(x) , y(y) {}

    double x;
    double y;
};

struct FilledCircle : IRenderElement {
    FilledCircle() : radius(0) {}

    FilledCircle(Point center, double radius, Color strokeColor, Color fillColor)
        : center(center)
        , radius(radius)
        , strokeColor(strokeColor)
        , fillColor(fillColor)
    {
    }

    void render(Cairo::RefPtr<Cairo::Context>& ctx) {
        ctx->save();
        strokeColor.render(ctx);
        ctx->arc(center.x, center.y, radius, 0, 2.0 * M_PI);
        ctx->stroke_preserve();
        fillColor.render(ctx);
        ctx->fill();
        ctx->restore();
    }

    Point center;
    double radius;
    Color strokeColor;
    Color fillColor;
};

struct Font : IRenderElement {
    Font(
            std::string const& face,
            double size,
            Cairo::FontWeight weight = Cairo::FontWeight::FONT_WEIGHT_NORMAL,
            Cairo::FontSlant slant = Cairo::FontSlant::FONT_SLANT_NORMAL 
            )
        : face(face)
        , size(size)
        , weight(weight)
        , slant(slant)
    {
    }

    Cairo::TextExtents text_extents(
            Cairo::RefPtr<Cairo::Context>& ctx,
            std::string const& s)
    {
        ctx->save();
        Cairo::TextExtents rv;
        render(ctx);
        ctx->get_text_extents(s, rv);
        ctx->restore();
        return rv;
    }

    void render(Cairo::RefPtr<Cairo::Context>& ctx) {
        ctx->select_font_face(face, slant, weight);
        ctx->set_font_size(size);
    }

    std::string face;
    double size;
    Cairo::FontWeight weight;
    Cairo::FontSlant slant;
};

struct Text : IRenderElement {
    enum Align {
        AlignLeft,
        AlignCenter,
        AlignRight
    };

    template<typename T>
    Text(
            Point pos,
            T txt,
            Font font,
            Align alignment,
            Color color = Color(0, 0, 0, 1)
            )
        : pos(pos)
        , txt(boost::lexical_cast<std::string>(txt))
        , font(font)
        , alignment(alignment)
        , color(color)
    {
    }

    void render(Cairo::RefPtr<Cairo::Context>& ctx) {
        ctx->save();

        font.render(ctx);
        setPosition(ctx);

        color.render(ctx);
        ctx->show_text(txt);
        ctx->stroke();
        ctx->restore();
    }

    Point pos;
    std::string txt;
    Font font;
    Align alignment;
    Color color;

private:
    void setPosition(Cairo::RefPtr<Cairo::Context>& ctx) {
        Cairo::TextExtents extents;
        switch (alignment) {
            case AlignLeft:
                ctx->move_to(pos.x, pos.y);
                break;

            case AlignCenter: {
                ctx->get_text_extents(txt, extents);

                double x = pos.x - extents.width / 2.0;
                double y = pos.y - extents.height / 2.0;

                ctx->move_to(x, y);
                }
                break;

            case AlignRight: {
                ctx->get_text_extents(txt, extents);

                double x = pos.x - extents.width;
                double y = pos.y - extents.height / 2.0;

                ctx->move_to(x, y);
                }
                break;


        }
    }
};
