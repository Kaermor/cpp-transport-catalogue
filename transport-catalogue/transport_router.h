#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <unordered_map>

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0;
};

class TransportRouter {
public:
    TransportRouter() = default;
    TransportRouter(const RoutingSettings& routing_settings)
        : routing_settings_(routing_settings) {}
    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue
                    , const RoutingSettings& routing_settings) {
        routing_settings_ = routing_settings;
        graph_ = BuildGraph(catalogue);
    }

    const graph::DirectedWeightedGraph<double>& BuildGraph(
                                            const transport_catalogue::TransportCatalogue& catalogue);
    const std::optional<graph::Router<double>::RouteInfo> FindRoute(
                                            const std::string_view from
                                            , const std::string_view to) const;
    const graph::DirectedWeightedGraph<double>& GetGraph() const;


private:
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unordered_map<std::string_view, graph::VertexId> stop_to_vertex_ids_;
    std::unique_ptr<graph::Router<double>> router_;

    void FillVertexes(const transport_catalogue::TransportCatalogue& catalogue);
    void FillEdges(const transport_catalogue::TransportCatalogue& catalogue);
};

} // namespace transport_router