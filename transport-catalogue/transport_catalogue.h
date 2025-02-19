#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
    int stops_count = 0;
    int unique_stops = 0;
    double route_length = 0.0;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const std::string& stop_name, const Coordinates& coordinates);

    void AddBus(const std::string& bus_id
                , const std::vector<std::string_view>& route_stops);

    const Stop* GetStop(const std::string_view stop_name) const;

    const Bus* GetBus(const std::string_view bus_id) const;

    const BusRouteInfo GetBusInfo(const std::string_view bus_id) const;

    const std::unordered_set<std::string_view> GetStopInfo(const std::string_view stop_id) const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> buses_at_stop_;
};

}
