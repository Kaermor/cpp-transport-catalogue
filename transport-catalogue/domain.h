#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace transport_catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> route_stops;
    bool is_roundtrip = false;
};

struct BusRouteInfo {
    double route_length = 0.0;
    double curvature = 1;
    int stops_count = 0;
    int unique_stops = 0;
};

} //namespace transport_catalogue