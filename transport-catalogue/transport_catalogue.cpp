#include "transport_catalogue.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace transport_catalogue {

size_t Stop2StopHasher::operator() (const std::pair<const Stop*, const Stop*>& stops_pair) const {
    size_t stop1_hash = std::hash<const void*>{}(stops_pair.first);
    size_t stop2_hash = std::hash<const void*>{}(stops_pair.second);
    return stop1_hash ^ (stop2_hash << 1);
}

void TransportCatalogue::AddStop(const std::string& stop_name
                                 , const geo::Coordinates& coordinates) {
    stops_.push_back({stop_name, coordinates});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::SetStop2StopDistance(const std::string_view stop_from
                                            , const std::string_view stop_to, int distance){
        auto stop_presence = stopname_to_stop_.find(stop_from);
        auto to_stop_presence = stopname_to_stop_.find(stop_to);
        if (stop_presence != stopname_to_stop_.end()
            && to_stop_presence != stopname_to_stop_.end()) {
            auto key = std::pair(stop_presence->second
                                 , to_stop_presence->second);
            stop2stop_distances_.insert({key, distance});
        }
}

void TransportCatalogue::AddBus(const std::string& bus_id
                                , const std::vector<std::string_view>& route_stops
                                , bool is_roundtrip) {
    std::vector<const Stop*> result;
    result.reserve(route_stops.size());
    buses_.push_back({bus_id, result, is_roundtrip});

    for (const auto& stop : route_stops) {
        auto stop_presence = stopname_to_stop_.find(stop);
        if (stop_presence == stopname_to_stop_.end()) {
            continue;
        }
        result.push_back(stop_presence->second);
        buses_at_stop_[stop_presence->first].insert(buses_.back().name);
    }

    buses_.back().route_stops = std::move(result);
    busname_to_bus_[buses_.back().name] = &buses_.back();
}

const Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
    auto stop_presence = stopname_to_stop_.find(stop_name);
    if (stop_presence == stopname_to_stop_.end()) {
        return nullptr;
    }
    return stop_presence->second;
}

const Bus* TransportCatalogue::GetBus(const std::string_view bus_id) const {
    auto bus_presence = busname_to_bus_.find(bus_id);
    if (bus_presence == busname_to_bus_.end()) {
        return nullptr;
    }
    return bus_presence->second;
}

const BusRouteInfo TransportCatalogue::GetBusInfo(const std::string_view bus_id) const {
    BusRouteInfo result;
    auto bus_ptr = GetBus(bus_id);

    if (!GetBus(bus_id)) {
        return result;
    }

    double route_geo_length = 0.0;
    result.stops_count = static_cast<int>(bus_ptr->route_stops.size());
    std::unordered_set<const Stop*> unique_stops(bus_ptr->route_stops.begin()
                                                  , bus_ptr->route_stops.end());
    result.unique_stops = static_cast<int>(unique_stops.size());

    if(!bus_ptr->is_roundtrip) {
        
        for (size_t i = 1; i < static_cast<size_t>(result.stops_count); ++i) {
            result.route_length += GetRealDistance(bus_ptr->route_stops[i-1]
                                                       , bus_ptr->route_stops[i]);
            route_geo_length += ComputeGeoDistance(bus_ptr->route_stops[i]->coordinates
                                                , bus_ptr->route_stops[i-1]->coordinates);
        }

        for (size_t i = static_cast<size_t>(result.stops_count) - 1; i > 0; --i) {
            result.route_length += GetRealDistance(bus_ptr->route_stops[i]
                                                       , bus_ptr->route_stops[i-1]);
            route_geo_length += ComputeGeoDistance(bus_ptr->route_stops[i]->coordinates
                                                , bus_ptr->route_stops[i-1]->coordinates);
        }
        
        result.stops_count = static_cast<int>(bus_ptr->route_stops.size()) * 2 - 1;
    } else {

        for (size_t i = 1; i < static_cast<size_t>(result.stops_count); ++i) {
            result.route_length += GetRealDistance(bus_ptr->route_stops[i-1]
                                                    , bus_ptr->route_stops[i]);
            route_geo_length += ComputeGeoDistance(bus_ptr->route_stops[i]->coordinates
                                                , bus_ptr->route_stops[i-1]->coordinates);
        }
    }

    result.curvature = result.route_length / route_geo_length;

    return result;
}

const std::unordered_set<std::string_view>& TransportCatalogue::GetStopInfo(
                                        const std::string_view stop_name) const {
    static const std::unordered_set<std::string_view> dummy;
    auto result = buses_at_stop_.find(stop_name);
    if (result == buses_at_stop_.end()) {
        return dummy;
    }
    return result->second;
}

const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetAllBuses() const {
    return busname_to_bus_;
}

int TransportCatalogue::GetRealDistance(const Stop* stop_from, const Stop* stop_to) const {
    auto it_forw = stop2stop_distances_.find({stop_from, stop_to});
    if (it_forw != stop2stop_distances_.end()) {
        return it_forw->second;
    }
    auto it_backw = stop2stop_distances_.find({stop_to, stop_from});
    if (it_backw != stop2stop_distances_.end()) {
        return it_backw->second;
    }
    return 0;
}

} // namespace transport_catalogue