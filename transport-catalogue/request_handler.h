#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include <optional>

class RequestHandler {
public:
RequestHandler(const transport_catalogue::TransportCatalogue& catalogue
                , const map_renderer::MapRenderer& renderer
                , const transport_router::TransportRouter& router)
    : catalogue_(catalogue)
    , renderer_(renderer)
    , router_(router) {}

    bool IsStopExist(const std::string_view stop_name) const;
    bool IsBusExist(const std::string_view bus_name) const;
    transport_catalogue::BusRouteInfo GetBusRouteInfo(
                                        const std::string_view& bus_name) const;
    const std::unordered_set<std::string_view>& GetBusesByStop(
                                        const std::string_view& stop_name) const;
    const std::optional<std::vector<graph::Edge<double>>> GetOptimalRoute(
                                                                        const std::string_view stop_from
                                                                        , const std::string_view stop_to) const;
    svg::Document RenderMap() const;

private:
    const transport_catalogue::TransportCatalogue& catalogue_;
    const map_renderer::MapRenderer& renderer_;
    const transport_router::TransportRouter& router_;
};
