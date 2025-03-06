#pragma once

#include "geo.h"

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport_catalogue {

struct Stop {
    std::string name;
    Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> route_stops;
};

struct BusRouteInfo {
    double route_length = 0.0;
    double curvature = 1;
    uint32_t stops_count = 0;
    uint32_t unique_stops = 0;
};

struct Stop2StopHasher {
    size_t operator() (const std::pair<const Stop*, const Stop*>& stops_pair) const;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const std::string& stop_name, const Coordinates& coordinates);

    void AddStop2StopDistance(const std::string_view stop_from, const std::string_view stop_to, uint32_t distance);

    void AddBus(const std::string& bus_id
                , const std::vector<std::string_view>& route_stops);

    const Stop* GetStop(const std::string_view stop_name) const;

    const Bus* GetBus(const std::string_view bus_id) const;

    const BusRouteInfo GetBusInfo(const std::string_view bus_id) const;

    const std::unordered_set<std::string_view>& GetStopInfo(const std::string_view stop_name) const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> buses_at_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, uint32_t, Stop2StopHasher> stop2stop_distances_;

    uint32_t ComputeRealDistance(const Stop* stop_from, const Stop* stop_to) const;
};

} // namespace transport_catalogue
