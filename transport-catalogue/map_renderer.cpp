#include "map_renderer.h"

namespace map_renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

std::vector<std::pair<std::string_view
, const transport_catalogue::Bus*>> MapRenderer::GetSortedAllBuses(
                                        const std::unordered_map<std::string_view
                                        , const transport_catalogue::Bus*>& all_buses) const {
    std::vector<std::pair<std::string_view, const transport_catalogue::Bus*>> result(
                                                            all_buses.begin(), all_buses.end());
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<svg::Polyline> MapRenderer::CreateSvgBusLines(
                                        const std::vector<std::pair<std::string_view
                                        , const transport_catalogue::Bus*>>& sorted_buses
                                        , const SphereProjector& sp) const {
    std::vector<svg::Polyline> result;
    size_t color_num = 0;
    for (const auto& [bus_id, bus] : sorted_buses) {
        if (bus->route_stops.empty()) continue;
        std::vector<const transport_catalogue::Stop*> route_stops{ bus->route_stops.begin()
                                                                , bus->route_stops.end() };
        if (!bus->is_roundtrip) {
            route_stops.insert(route_stops.end(), std::next(bus->route_stops.rbegin())
                                                                , bus->route_stops.rend());
        }
        svg::Polyline line;
        for (const auto& stop : route_stops) {
            line.AddPoint(sp(stop->coordinates));
        }
        line.SetStrokeColor(render_settings_.color_palette[color_num]);
        line.SetFillColor("none");
        line.SetStrokeWidth(render_settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        color_num < (render_settings_.color_palette.size() - 1) ? ++color_num : color_num = 0;

        result.push_back(line);
    }
    return result;
}

std::vector<svg::Text> MapRenderer::CreateSvgBusLabels(
                                                const std::vector<std::pair<std::string_view
                                                , const transport_catalogue::Bus*>>& sorted_buses
                                                , const SphereProjector& sp) const {
    std::vector<svg::Text> result;
    size_t color_num = 0;

    for (const auto& [bus_id, bus] : sorted_buses) {
        if (bus->route_stops.empty()) continue;
        svg::Text bus_label;
        svg::Text bus_underlayer;
        bus_label.SetPosition(sp(bus->route_stops[0]->coordinates));
        bus_label.SetOffset(render_settings_.bus_label_offset);
        bus_label.SetFontSize(
                    static_cast<uint32_t>(render_settings_.bus_label_font_size));
        bus_label.SetFontFamily("Verdana");
        bus_label.SetFontWeight("bold");
        bus_label.SetData(bus->name);
        bus_label.SetFillColor(render_settings_.color_palette[color_num]);
        
        color_num < (render_settings_.color_palette.size() - 1) ? ++color_num : color_num = 0;

        bus_underlayer.SetPosition(sp(bus->route_stops[0]->coordinates));
        bus_underlayer.SetOffset(render_settings_.bus_label_offset);
        bus_underlayer.SetFontSize(
                    static_cast<uint32_t>(render_settings_.bus_label_font_size));
        bus_underlayer.SetFontFamily("Verdana");
        bus_underlayer.SetFontWeight("bold");
        bus_underlayer.SetData(bus->name);
        bus_underlayer.SetFillColor(render_settings_.underlayer_color);
        bus_underlayer.SetStrokeColor(render_settings_.underlayer_color);
        bus_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        bus_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        bus_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        result.push_back(bus_underlayer);
        result.push_back(bus_label);

        if (!bus->is_roundtrip && bus->route_stops[0] 
            != bus->route_stops[bus->route_stops.size() - 1]) {
            svg::Text bus_label_end {bus_label};
            svg::Text bus_underlayer_end {bus_underlayer};
            bus_label_end.SetPosition(
                sp(bus->route_stops[bus->route_stops.size() - 1]->coordinates));
            bus_underlayer_end.SetPosition(
                sp(bus->route_stops[bus->route_stops.size() - 1]->coordinates));
            result.push_back(bus_underlayer_end);
            result.push_back(bus_label_end);
        }
    }

    return result;
}

    std::vector<svg::Circle> MapRenderer::CreateSvgStopSymbols(
                                        std::map<std::string_view
                                        , const transport_catalogue::Stop*>& all_routes_stops
                                        , const SphereProjector& sp) const {
        std::vector<svg::Circle> result;

        for (const auto& [stop_name, stop] : all_routes_stops) {
            svg::Circle symbol;
            symbol.SetCenter(sp(stop->coordinates));
            symbol.SetRadius(render_settings_.stop_radius);
            symbol.SetFillColor("white");
            result.push_back(symbol);
        }
    
        return result;
    }

    std::vector<svg::Text> MapRenderer::CreateSvgStopLabels(
                                        std::map<std::string_view
                                        , const transport_catalogue::Stop*>& all_routes_stops
                                        , const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        svg::Text stop_label;
        svg::Text stop_underlayer;

        for (const auto& [stop_name, stop] : all_routes_stops) {
            stop_label.SetPosition(sp(stop->coordinates));
            stop_label.SetOffset(render_settings_.stop_label_offset);
            stop_label.SetFontSize(
                    static_cast<uint32_t>(render_settings_.stop_label_font_size));
            stop_label.SetFontFamily("Verdana");
            stop_label.SetData(stop->name);
            stop_label.SetFillColor("black");
    
            stop_underlayer.SetPosition(sp(stop->coordinates));
            stop_underlayer.SetOffset(render_settings_.stop_label_offset);
            stop_underlayer.SetFontSize(
                    static_cast<uint32_t>(render_settings_.stop_label_font_size));
            stop_underlayer.SetFontFamily("Verdana");
            stop_underlayer.SetData(stop->name);
            stop_underlayer.SetFillColor(render_settings_.underlayer_color);
            stop_underlayer.SetStrokeColor(render_settings_.underlayer_color);
            stop_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            stop_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            stop_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
            result.push_back(stop_underlayer);
            result.push_back(stop_label);
        }
    
        return result;
    }

svg::Document MapRenderer::CreateSvgDoc(const std::unordered_map<std::string_view
    , const transport_catalogue::Bus*>& all_buses) const {
        svg::Document result;
        std::vector<geo::Coordinates> route_stops_coord;
        std::vector<std::pair<std::string_view
                            , const transport_catalogue::Bus*>> sorted_buses
                                                 = GetSortedAllBuses(all_buses);
        std::map<std::string_view, const transport_catalogue::Stop*> all_routes_stops;
        
        for (const auto& [bus_id, bus] : sorted_buses) {
            if (bus->route_stops.empty()) {
                continue;
            }
            for (const auto& stop : bus->route_stops) {
                route_stops_coord.push_back(stop->coordinates);
                all_routes_stops.emplace(stop->name, stop);
            }
        }

        SphereProjector sp(route_stops_coord.begin()
                            , route_stops_coord.end()
                            , render_settings_.width
                            , render_settings_.height
                            , render_settings_.padding);

        for (const auto& line : CreateSvgBusLines(sorted_buses, sp)) {
            result.Add(line);
        }
        for (const auto& label : CreateSvgBusLabels(sorted_buses, sp)) {
            result.Add(label);
        }
        for (const auto& symbol : CreateSvgStopSymbols(all_routes_stops, sp)) {
            result.Add(symbol);
        }
        for (const auto& label : CreateSvgStopLabels(all_routes_stops, sp)) {
            result.Add(label);
        }

        return result;
}

void MapRenderer::PrintMap(svg::Document& rendered_map, std::ostream& output) const {
    rendered_map.Render(output);
}

} // namespace map_renderer