#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

    namespace detail {

        template <typename T>
        inline void RenderValue(std::ostream& out, const T& value) {
            out << value;
        }

        void HtmlEncodeString(std::ostream& out, std::string_view sv);

        template <>
        inline void RenderValue<std::string>(std::ostream& out, const std::string& s) {
            HtmlEncodeString(out, s);
        }

        template <typename AttrType>
        inline void RenderAttr(std::ostream& out, std::string_view name, const AttrType& value) {
            using namespace std::literals;
            out << name << "=\""sv;
            RenderValue(out, value);
            out.put('"');
        }

        template <typename AttrType>
        inline void RenderOptionalAttr(std::ostream& out, std::string_view name,
                                       const std::optional<AttrType>& value) {
            if (value) {
                RenderAttr(out, name, *value);
            }
        }

    }  // namespace detail

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
    
    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

struct Point {
    Point() = default;
    Point(double xx, double yy)
        : x(xx)
        , y(yy) {
    }
    double x = 0;
    double y = 0;
};

struct Rgb {
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r)
        , green(g)
        , blue(b) {
    }
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
        : red(r)
        , green(g)
        , blue(b)
        , opacity(op) {
    }
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{std::monostate{}};

std::ostream& operator<<(std::ostream& out, const Color& color);

struct ColorPrinter {
    std::ostream& out;
    void operator()(std::monostate) const;
    void operator()(const std::string& color) const;
    void operator()(const Rgb& color) const;
    void operator()(const Rgba& color) const;
};

struct RenderContext {
    RenderContext(std::ostream& outt)
        : out(outt) {
    }

    RenderContext(std::ostream& outt, int indnt_step, int indnt = 0)
        : out(outt)
        , indent_step(indnt_step)
        , indent(indnt) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using detail::RenderOptionalAttr;
        using namespace std::literals;
        RenderOptionalAttr(out, " fill"sv, fill_color_);
        RenderOptionalAttr(out, " stroke"sv, stroke_color_);
        RenderOptionalAttr(out, " stroke-width"sv, stroke_width_);
        RenderOptionalAttr(out, " stroke-linecap"sv, stroke_line_cap_);
        RenderOptionalAttr(out, " stroke-linejoin"sv, stroke_line_join_);
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;
    Point center_;
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    Point pos_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_; 
};

class ObjectContainer {
public:
template <typename ObjectType>
    void Add(ObjectType object) {
        AddPtr(std::make_unique<ObjectType>(std::move(object)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    ~ObjectContainer() = default;
};

class Drawable {
    public:
        virtual ~Drawable() = default; 
        virtual void Draw(ObjectContainer& container) const = 0;
    };

class Document : public ObjectContainer {
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg