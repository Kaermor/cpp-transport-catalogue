#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

class RequestHandler {
public:
RequestHandler(const transport_catalogue::TransportCatalogue& catalogue
                , const map_renderer::MapRenderer& renderer)
    : catalogue_(catalogue)
    , renderer_(renderer) {}

    bool IsStopExist(const std::string_view& stop_name) const;
    bool IsBusExist(const std::string_view& bus_name) const;
    transport_catalogue::BusRouteInfo GetBusRouteInfo(
                                        const std::string_view& bus_name) const;
    const std::unordered_set<std::string_view>& GetBusesByStop(
                                        const std::string_view& stop_name) const;
    svg::Document RenderMap() const;

private:
    const transport_catalogue::TransportCatalogue& catalogue_;
    const map_renderer::MapRenderer& renderer_;
};

