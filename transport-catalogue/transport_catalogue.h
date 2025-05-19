#pragma once

#include "domain.h"
#include "geo.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport_catalogue {

struct Stop2StopHasher {
    size_t operator() (const std::pair<const Stop*, const Stop*>& stops_pair) const;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const std::string& stop_name, const geo::Coordinates& coordinates);

    void SetStop2StopDistance(const std::string_view stop_from, const std::string_view stop_to, int distance);

    void AddBus(const std::string& bus_id
                , const std::vector<std::string_view>& route_stops
                , bool is_roundtrip);

    const Stop* GetStop(const std::string_view stop_name) const;

    const Bus* GetBus(const std::string_view bus_id) const;

    const BusRouteInfo GetBusInfo(const std::string_view bus_id) const;

    const std::unordered_set<std::string_view>& GetStopInfo(const std::string_view stop_name) const;

    const std::unordered_map<std::string_view, const Bus*>& GetAllBuses() const;

    const std::unordered_map<std::string_view, const Stop*>& GetAllStops() const;

    int GetRealDistance(const Stop* stop_from, const Stop* stop_to) const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> buses_at_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, Stop2StopHasher> stop2stop_distances_;
 
};

} // namespace transport_catalogue