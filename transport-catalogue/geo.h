#pragma once

namespace geo {

struct Coordinates {
    Coordinates() = default;
    Coordinates(double latt, double lngt)
    : lat(latt), lng(lngt) {}
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

double ComputeGeoDistance(Coordinates from, Coordinates to);

}  // namespace geo