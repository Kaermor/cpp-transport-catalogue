#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>
#include <sstream>

namespace json_reader {

struct StopDescription {
    std::string stop_name;
    geo::Coordinates coordinates;
    std::unordered_map<std::string, int> stop2stop_distances;
};

struct BusDescription {
    std::string bus_name;
    std::vector<std::string_view> route_stops;
    bool is_roundtrip = false;
};

class JsonReader {
public:
    JsonReader() = default;
    JsonReader(std::istream& input)
        : doc_(json::Load(input)) {}

    const json::Node& GetBaseRequests() const;
    const json::Node& GetRenderSettings() const;
    const json::Node& GetStatRequests() const;

    void FillStops(transport_catalogue::TransportCatalogue& catalogue);
    void SetStopsDistances(transport_catalogue::TransportCatalogue& catalogue);
    void FillBuses(transport_catalogue::TransportCatalogue& catalogue);    
    void FillTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue);

    map_renderer::MapRenderer SetRenderSettings(const json::Dict& render_settings) const;

    const json::Node ProcessBusRequest(const json::Dict& request, RequestHandler& rh) const;
    const json::Node ProcessStopRequest(const json::Dict& request, RequestHandler& rh) const;
    const json::Node ProcessMapRequest(const json::Dict& request, RequestHandler& rh) const;   
    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const;


private:
    json::Document doc_;
};

}  // namespace json_reader