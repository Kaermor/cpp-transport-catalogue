#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}

void ColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}

void ColorPrinter::operator()(const std::string& color) const {
    out << color;
}

void ColorPrinter::operator()(const Rgb& color) const {
    out << "rgb("sv << static_cast<int>(color.red) << ','
                    << static_cast<int>(color.green) << ','
                    << static_cast<int>(color.blue) << ')';
}

void ColorPrinter::operator()(const Rgba& color) const {
    out << "rgba("sv << static_cast<int>(color.red) << ','
                    << static_cast<int>(color.green) << ','
                    << static_cast<int>(color.blue) << ','
                    << static_cast<double>(color.opacity) << ')';
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool isFirst = true;
    for (const Point& point : points_) {
        if (isFirst) {
            isFirst = false;
        } else {
            out << ' ';
        }
        out << point.x << ',' << point.y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    using detail::RenderAttr;
    RenderAttr(out, " x"sv, pos_.x);
    RenderAttr(out, " y"sv, pos_.y);
    RenderAttr(out, " dx"sv, offset_.x);
    RenderAttr(out, " dy"sv, offset_.y);
    RenderAttr(out, " font-size"sv, font_size_);
    if (!font_family_.empty()) {
        RenderAttr(out, " font-family"sv, font_family_);
    }
    if (!font_weight_.empty()) {
        RenderAttr(out, " font-weight"sv, font_weight_);
    }
    out.put('>');
    detail::HtmlEncodeString(out, data_);
    out << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }

    out << "</svg>"sv;
}

namespace detail {

    void HtmlEncodeString(std::ostream& out, std::string_view sv) {
        for (char c : sv) {
            switch (c) {
                case '"':
                    out << "&quot;"sv;
                    break;
                case '<':
                    out << "&lt;"sv;
                    break;
                case '>':
                    out << "&gt;"sv;
                    break;
                case '&':
                    out << "&amp;"sv;
                    break;
                case '\'':
                    out << "&apos;"sv;
                    break;
                default:
                    out.put(c);
            }
        }
    }

}  // namespace detail

}  // namespace svg