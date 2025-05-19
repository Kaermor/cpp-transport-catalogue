#include "json_reader.h"
#include "json_builder.h"

#include <algorithm>

using namespace std::literals;

namespace json_reader {

const json::Node& JsonReader::GetBaseRequests() const {
    return doc_.GetRoot().AsDict().at("base_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    return doc_.GetRoot().AsDict().at("render_settings");
}

const json::Node& JsonReader::GetStatRequests() const {
    return doc_.GetRoot().AsDict().at("stat_requests");
}

const json::Node& JsonReader::GetRoutingSettings() const {
    return doc_.GetRoot().AsDict().at("routing_settings");
}

void JsonReader::FillStops(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& base_requests = GetBaseRequests().AsArray();
    for (const auto& request : base_requests) {
        if (request.IsDict()) {
            const auto& request_typed = request.AsDict();
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
        if (request.IsDict()) {
            const auto& request_typed = request.AsDict();
            if (request_typed.at("type").AsString() == "Stop") {
                for (const auto& [stop_name, distance] : request_typed.at("road_distances").AsDict()) {
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
        if (request.IsDict()) {
            const auto& request_typed = request.AsDict();
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
    json::Node result = json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(request_id)
                            .Key("error_message"s).Value("not found"s)
                        .EndDict()
                .Build();
    return result;
}

svg::Color JsonReader::GetColorInRightFormat(const json::Node& color_setting) const {
    svg::Color result_color;
    if (color_setting.IsString()) {
        result_color = color_setting.AsString();
    } else if (color_setting.IsArray()) {
        const auto& color = color_setting.AsArray();
        if (color.size() == 3) {
            result_color = svg::Rgb(static_cast<uint8_t>(color[0].AsInt())
                                    , static_cast<uint8_t>(color[1].AsInt())
                                    , static_cast<uint8_t>(color[2].AsInt()));
        } else if (color.size() == 4) {
            result_color = svg::Rgba(static_cast<uint8_t>(color[0].AsInt())
                                    , static_cast<uint8_t>(color[1].AsInt())
                                    , static_cast<uint8_t>(color[2].AsInt())
                                    , color[3].AsDouble());
        }
    }
    return result_color;
}

map_renderer::MapRenderer JsonReader::SetRenderSettings(const json::Node& render_settings) const {
    map_renderer::RenderSettings result_settings;
    json::Dict render_settings_dict = render_settings.AsDict();
    result_settings.width = render_settings_dict.at("width").AsDouble();
    result_settings.height = render_settings_dict.at("height").AsDouble();
    result_settings.padding = render_settings_dict.at("padding").AsDouble();
    result_settings.line_width = render_settings_dict.at("line_width").AsDouble();
    result_settings.stop_radius = render_settings_dict.at("stop_radius").AsDouble();
    result_settings.bus_label_font_size = render_settings_dict.at("bus_label_font_size").AsInt();
    result_settings.bus_label_offset = {render_settings_dict.at("bus_label_offset")
                                                            .AsArray()[0].AsDouble()
                                        , render_settings_dict.at("bus_label_offset")
                                                            .AsArray()[1].AsDouble()};
    result_settings.stop_label_font_size = render_settings_dict.at("stop_label_font_size").AsInt();
    result_settings.stop_label_offset = {render_settings_dict.at("stop_label_offset")
                                                            .AsArray()[0].AsDouble()
                                        , render_settings_dict.at("stop_label_offset")
                                                            .AsArray()[1].AsDouble()};
    result_settings.underlayer_color
                    = GetColorInRightFormat(render_settings_dict.at("underlayer_color"));
    result_settings.underlayer_width = render_settings_dict.at("underlayer_width").AsDouble();

    const auto& color_palette = render_settings_dict.at("color_palette").AsArray();
    
    for (const auto& color : color_palette) {
        result_settings.color_palette.push_back(GetColorInRightFormat(color));
    }
    
    return result_settings;
}

transport_router::RoutingSettings JsonReader::SetRoutingSettings(const json::Node& routing_settings) const {
    transport_router::RoutingSettings result_settings;
    json::Dict routing_settings_dict = routing_settings.AsDict();
    double kmph_to_mpm = 1000.0 / 60.0;
    result_settings.bus_velocity = routing_settings_dict.at("bus_velocity").AsDouble() * kmph_to_mpm;
    result_settings.bus_wait_time = routing_settings_dict.at("bus_wait_time").AsInt();
    return result_settings;
}

const json::Node JsonReader::ProcessBusRequest(const json::Dict& request
                                                , RequestHandler& rh) const {
    if (!rh.IsBusExist(request.at("name").AsString())) {
        return ProcessErrorRequest(request.at("id").AsInt());
    }
    const auto bus_route_info = rh.GetBusRouteInfo(request.at("name").AsString());
    json::Node result = json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(request.at("id").AsInt())
                            .Key("curvature"s).Value(bus_route_info.curvature)
                            .Key("route_length"s).Value(bus_route_info.route_length)
                            .Key("stop_count"s).Value(bus_route_info.stops_count)
                            .Key("unique_stop_count"s).Value(bus_route_info.unique_stops)
                        .EndDict()
                .Build();
    return result;
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
    json::Array buses_array;

    for (const auto& bus : vec_buses_at_stop) {
        buses_array.emplace_back(std::string(bus));
    }

    json::Node result = json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(request.at("id").AsInt())
                            .Key("buses"s).Value(buses_array)
                        .EndDict()
                .Build();
    return result;
}

const json::Node JsonReader::ProcessMapRequest(const json::Dict& map_request
                                                , RequestHandler& rh) const {
    std::ostringstream out_stream;
    svg::Document rendered_map = rh.RenderMap();
    rendered_map.Render(out_stream);  
    json::Node result = json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(map_request.at("id").AsInt())
                            .Key("map"s).Value(out_stream.str())
                        .EndDict()
                .Build();
    return result;
}

const json::Node JsonReader::ProcessRouteRequest(const json::Dict& request
                                                , RequestHandler& rh) const {
    const auto route = rh.GetOptimalRoute(request.at("from").AsString()
                                            , request.at("to").AsString());
    if (!route) {
        return ProcessErrorRequest(request.at("id").AsInt());
    }

    json::Array items_array;
    double total_time = 0.0;
    for (const auto& item_edge : route.value().edges) {
        const graph::Edge<double> edge = rh.GetRouterGraph().GetEdge(item_edge);
            if (edge.span_count == 0) {
                items_array.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key("stop_name"s).Value(edge.name)
                        .Key("time"s).Value(edge.weight)
                        .Key("type"s).Value("Wait"s)
                    .EndDict()
                .Build()));

                total_time += edge.weight;
            }
            else {
                items_array.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key("bus"s).Value(edge.name)
                        .Key("span_count"s).Value(static_cast<int>(edge.span_count))
                        .Key("time"s).Value(edge.weight)
                        .Key("type"s).Value("Bus"s)
                    .EndDict()
                .Build()));

                total_time += edge.weight;
            }
    }
    json::Node result = json::Builder{}
    .StartDict()
        .Key("request_id"s).Value(request.at("id"s).AsInt())
        .Key("total_time"s).Value(total_time)
        .Key("items"s).Value(items_array)
    .EndDict()
    .Build();

    return result;
}

void JsonReader::ProcessRequests(const json::Node& stat_requests
                                    , RequestHandler& rh) const {
    json::Array result;                                
    const json::Array& stat_requests_array = stat_requests.AsArray();

    for (const auto& request : stat_requests_array) {
        if (request.IsDict()) {
            const auto& request_typed = request.AsDict();
            if (request_typed.at("type").AsString() == "Bus") {
                result.emplace_back(ProcessBusRequest(request_typed, rh));
            }
            if (request_typed.at("type").AsString() == "Stop") {
                result.emplace_back(ProcessStopRequest(request_typed, rh));
            }
            if (request_typed.at("type").AsString() == "Map") {
                result.emplace_back(ProcessMapRequest(request_typed, rh));
            }
            if (request_typed.at("type").AsString() == "Route") {
                result.emplace_back(ProcessRouteRequest(request_typed, rh));
            }
        }
    }

    json::Print(json::Document{result}, std::cout);
}

}  // namespace json_reader
