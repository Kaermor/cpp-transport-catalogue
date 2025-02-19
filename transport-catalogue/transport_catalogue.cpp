#include "transport_catalogue.h"

#include <unordered_set>

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string& stop_name
                                 , const Coordinates& coordinates) {
    stops_.push_back({stop_name, coordinates});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(const std::string& bus_id
                                , const std::vector<std::string_view>& route_stops) {
    std::vector<const Stop*> result;
    result.reserve(route_stops.size());
    buses_.push_back({bus_id, result});

    for (const auto& stop : route_stops) {
        if (!stopname_to_stop_.count(stop)) {
            continue;
        }
        result.push_back(const_cast<Stop*>(stopname_to_stop_.at(stop)));
        buses_at_stop_[stopname_to_stop_.at(stop)->name].insert(buses_.back().name);
    }

    buses_.back().route_stops = std::move(result);
    busname_to_bus_[buses_.back().name] = &buses_.back();
}

const Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
    return stopname_to_stop_.count(stop_name) > 0 ? stopname_to_stop_.at(stop_name) : nullptr;
}

const Bus* TransportCatalogue::GetBus(const std::string_view bus_id) const {
    return busname_to_bus_.count(bus_id) > 0 ? busname_to_bus_.at(bus_id) : nullptr;
}

const BusRouteInfo TransportCatalogue::GetBusInfo(const std::string_view bus_id) const {
    BusRouteInfo result;
    auto bus_ptr = GetBus(bus_id);

    if (!GetBus(bus_id)) {
        return result;
    }

    result.stops_count = bus_ptr->route_stops.size();
    std::unordered_set<const Stop*> unique_stops(bus_ptr->route_stops.begin()
                                                  , bus_ptr->route_stops.end());
    result.unique_stops = unique_stops.size();

    for (size_t i = 1; i < result.stops_count; ++i) {
        result.route_length += ComputeDistance(bus_ptr->route_stops[i]->coordinates
                                            , bus_ptr->route_stops[i-1]->coordinates);
    }

    return result;
}

const std::unordered_set<std::string_view> TransportCatalogue::GetStopInfo(const std::string_view stop_name) const {
    static const std::unordered_set<std::string_view> dummy;
    auto result = buses_at_stop_.find(stop_name);
    if (result == buses_at_stop_.end()) {
        return dummy;
    }
    return result->second;
}

}
