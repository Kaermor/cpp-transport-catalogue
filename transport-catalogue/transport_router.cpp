#include "transport_router.h"

namespace transport_router {

void TransportRouter::FillVertexes(const transport_catalogue::TransportCatalogue& catalogue) {
    const auto& all_stops = catalogue.GetAllStops();
    graph_ = graph::DirectedWeightedGraph<double>(all_stops.size() * 2);
    graph::VertexId vertex_id = 0;

    for (const auto& [stop_name, stop_ptr] : all_stops) {
        stop_to_vertex_ids_[stop_ptr->name] = vertex_id;
        graph_.AddEdge({stop_ptr->name
                    , 0
                    , vertex_id++
                    , vertex_id++
                    , static_cast<double>(routing_settings_.bus_wait_time)});
    }
}

void TransportRouter::FillEdges(const transport_catalogue::TransportCatalogue& catalogue) {
    const auto& all_buses = catalogue.GetAllBuses();

    for (const auto& [bus_name, bus_info] : all_buses) {
        const auto& stops = bus_info->route_stops;
        size_t stops_count = stops.size();

        for (size_t i = 0; i < stops_count; ++i) {
            int dist_sum = 0;
            int dist_sum_inverse = 0;

            for (size_t j = i + 1; j < stops_count; ++j) {
                const transport_catalogue::Stop* stop_from = stops[i];
                const transport_catalogue::Stop* stop_to = stops[j];

                dist_sum += catalogue.GetRealDistance(stops[j - 1], stops[j]);
                dist_sum_inverse += catalogue.GetRealDistance(stops[j], stops[j - 1]);

                graph_.AddEdge({ bus_info->name,
                                j - i,
                                stop_to_vertex_ids_.at(stop_from->name) + 1,
                                stop_to_vertex_ids_.at(stop_to->name),
                                static_cast<double>(dist_sum)
                                    / routing_settings_.bus_velocity });

                if (!bus_info->is_roundtrip) {
                    graph_.AddEdge({ bus_info->name,
                                    j - i,
                                    stop_to_vertex_ids_.at(stop_to->name) + 1,
                                    stop_to_vertex_ids_.at(stop_from->name),
                                    static_cast<double>(dist_sum_inverse)
                                        / routing_settings_.bus_velocity });
                }
            }
        }
    }
}

void TransportRouter::BuildGraph(
                    const transport_catalogue::TransportCatalogue& catalogue) {
    FillVertexes(catalogue);
    FillEdges(catalogue);
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

const std::optional<std::vector<graph::Edge<double>>> TransportRouter::FindRoute (
                                                    const std::string_view stop_from
                                                    , const std::string_view stop_to) const {
const auto route = router_->BuildRoute(stop_to_vertex_ids_.at(stop_from)
                                        , stop_to_vertex_ids_.at(stop_to));
    if (!route) {
        return std::nullopt;
    }

    std::optional<std::vector<graph::Edge<double>>> result
                                            = std::vector<graph::Edge<double>>{};
    result->reserve(route.value().edges.size());

    for (const auto& item_edge : route.value().edges) {
        const graph::Edge<double> edge = graph_.GetEdge(item_edge);
        result->push_back(edge);
    }

    return result;
}
} // namespace transport_router