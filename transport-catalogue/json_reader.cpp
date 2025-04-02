#include "json_reader.h"

#include <algorithm>

using namespace std::literals;

namespace json_reader {

const json::Node& JsonReader::GetBaseRequests() const {
    return doc_.GetRoot().AsMap().at("base_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    return doc_.GetRoot().AsMap().at("render_settings");
}

const json::Node& JsonReader::GetStatRequests() const {
    return doc_.GetRoot().AsMap().at("stat_requests");
}

void JsonReader::FillStops(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& base_requests = GetBaseRequests().AsArray();
    for (const auto& request : base_requests) {
        if (request.IsMap()) {
            const auto& request_typed = request.AsMap();
            if (request_typed.at("type").AsString() == "Bus") {
                continue;
            }
            StopDescription stop_description;
            stop_description.stop_name = request_typed.at("name").AsString();
            stop_description.coordinates.lat = request_typed.at("latitude").AsDouble();
            stop_description.coordinates.lng = request_typed.at("longitude").AsDouble();
            catalogue.AddStop(stop_description.stop_name, stop_description.coordinates);
        }
    }
}

void JsonReader::SetStopsDistances(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& base_requests = GetBaseRequests().AsArray();
    for (const auto& request : base_requests) {
        if (request.IsMap()) {
            const auto& request_typed = request.AsMap();
            if (request_typed.at("type").AsString() == "Stop") {
                for (const auto& [stop_name, distance] : request_typed.at("road_distances").AsMap()) {
                    catalogue.SetStop2StopDistance(request_typed.at("name").AsString()
                                                    , stop_name, distance.AsInt());
                }
            }
        }
    }
}

void JsonReader::FillBuses(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& base_requests = GetBaseRequests().AsArray();
    for (const auto& request : base_requests) {
        if (request.IsMap()) {
            const auto& request_typed = request.AsMap();
            if (request_typed.at("type").AsString() == "Stop") {
                continue;
            }
            BusDescription bus_description;
            bus_description.bus_name = request_typed.at("name").AsString();
            bus_description.is_roundtrip = request_typed.at("is_roundtrip").AsBool();
            const auto stops = request_typed.at("stops").AsArray();
            bus_description.route_stops.reserve(stops.size());

            for (size_t i = 0; i < stops.size(); ++i) {
                bus_description.route_stops.push_back(stops[i].AsString());
            }

            catalogue.AddBus(bus_description.bus_name
                            , bus_description.route_stops
                            , bus_description.is_roundtrip);
        }
    }
}

void JsonReader::FillTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
    FillStops(catalogue);
    SetStopsDistances(catalogue);
    FillBuses(catalogue);
}

json::Node ProcessErrorRequest(int request_id) {
    json::Dict result;
    result["request_id"s] = request_id;
    result["error_message"s] = "not found"s;
    return json::Node(result);
}

map_renderer::MapRenderer JsonReader::SetRenderSettings(const json::Dict& render_settings) const {
    map_renderer::RenderSettings result_settings;
    result_settings.width = render_settings.at("width").AsDouble();
    result_settings.height = render_settings.at("height").AsDouble();
    result_settings.padding = render_settings.at("padding").AsDouble();
    result_settings.line_width = render_settings.at("line_width").AsDouble();
    result_settings.stop_radius = render_settings.at("stop_radius").AsDouble();
    result_settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    result_settings.bus_label_offset = {render_settings.at("bus_label_offset")
                                                            .AsArray()[0].AsDouble()
                                        , render_settings.at("bus_label_offset")
                                                            .AsArray()[1].AsDouble()};
    result_settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    result_settings.stop_label_offset = {render_settings.at("stop_label_offset")
                                                            .AsArray()[0].AsDouble()
                                        , render_settings.at("stop_label_offset")
                                                            .AsArray()[1].AsDouble()};

    if (render_settings.at("underlayer_color").IsString()) {
        result_settings.underlayer_color = render_settings.at("underlayer_color").AsString();
    } else if (render_settings.at("underlayer_color").IsArray()) {
        const auto& color = render_settings.at("underlayer_color").AsArray();
        if (color.size() == 3) {
            result_settings.underlayer_color = svg::Rgb(color[0].AsInt()
                                                        , color[1].AsInt()
                                                        , color[2].AsInt());
        } else if (color.size() == 4) {
            result_settings.underlayer_color = svg::Rgba(color[0].AsInt()
                                                        , color[1].AsInt()
                                                        , color[2].AsInt()
                                                        , color[3].AsDouble());
        }
    }

    result_settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    const auto& color_palette = render_settings.at("color_palette").AsArray();
    for (const auto& color : color_palette) {
        if (color.IsString()) {
            result_settings.color_palette.push_back(color.AsString());
        } else if (color.IsArray()) {
            const auto& rgb = color.AsArray();
            if (rgb.size() == 3) {
                result_settings.color_palette.push_back(svg::Rgb(rgb[0].AsInt()
                                                                , rgb[1].AsInt()
                                                                , rgb[2].AsInt()));
            } else if (rgb.size() == 4) {
                result_settings.color_palette.push_back(svg::Rgba(rgb[0].AsInt()
                                                                , rgb[1].AsInt()
                                                                , rgb[2].AsInt()
                                                                , rgb[3].AsDouble()));
            }
        }
    }
    
    return result_settings;
}

const json::Node JsonReader::ProcessBusRequest(const json::Dict& request
                                                , RequestHandler& rh) const {
    if (!rh.IsBusExist(request.at("name").AsString())) {
        return ProcessErrorRequest(request.at("id").AsInt());
    }
    const auto bus_route_info = rh.GetBusRouteInfo(request.at("name").AsString());
    json::Dict result;
    result.emplace("request_id"s, request.at("id").AsInt());
    result.emplace("curvature"s, bus_route_info.curvature);
    result.emplace("route_length"s, bus_route_info.route_length);
    result.emplace("stop_count"s, bus_route_info.stops_count);
    result.emplace("unique_stop_count"s, bus_route_info.unique_stops);
    return json::Node(result);
}

const json::Node JsonReader::ProcessStopRequest(const json::Dict& request
                                                , RequestHandler& rh) const {
    if (!rh.IsStopExist(request.at("name").AsString())) {
        return ProcessErrorRequest(request.at("id").AsInt());
    }

    const auto set_buses_at_stop = rh.GetBusesByStop(request.at("name").AsString());
    std::vector<std::string_view> vec_buses_at_stop(set_buses_at_stop.begin()
                                                    , set_buses_at_stop.end());
    std::sort(vec_buses_at_stop.begin(), vec_buses_at_stop.end());
    json::Dict result;
    result.emplace("request_id"s, request.at("id").AsInt());
    json::Array buses_array;

    for (const auto& bus : vec_buses_at_stop) {
        buses_array.emplace_back(std::string(bus));
    }

    result.emplace("buses"s, buses_array);
    return json::Node(result);
}

const json::Node JsonReader::ProcessMapRequest(const json::Dict& map_request
                                                , RequestHandler& rh) const {
    json::Dict result;
    result["request_id"] = map_request.at("id").AsInt();
    std::ostringstream out_stream;
    svg::Document rendered_map = rh.RenderMap();
    rendered_map.Render(out_stream);
    result["map"] = out_stream.str();
    return json::Node{ result };    
}

void JsonReader::ProcessRequests(const json::Node& stat_requests
                                    , RequestHandler& rh) const {
    json::Array result;                                
    const json::Array& stat_requests_array = stat_requests.AsArray();

    for (const auto& request : stat_requests_array) {
        if (request.IsMap()) {
            const auto& request_typed = request.AsMap();
            if (request_typed.at("type").AsString() == "Bus") {
                result.emplace_back(ProcessBusRequest(request_typed, rh));
            }
            if (request_typed.at("type").AsString() == "Stop") {
                result.emplace_back(ProcessStopRequest(request_typed, rh));
            }
            if (request_typed.at("type").AsString() == "Map") {
                result.emplace_back(ProcessMapRequest(request_typed, rh));
            }
        }
    }

    json::Print(json::Document{result}, std::cout);
}

}  // namespace json_reader
