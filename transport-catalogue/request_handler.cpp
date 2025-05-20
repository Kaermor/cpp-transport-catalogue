#include "request_handler.h"

bool RequestHandler::IsStopExist(const std::string_view stop_name) const {
    return catalogue_.GetStop(stop_name) != nullptr;
}

bool RequestHandler::IsBusExist(const std::string_view bus_name) const {
    return catalogue_.GetBus(bus_name) != nullptr;
}

transport_catalogue::BusRouteInfo RequestHandler::GetBusRouteInfo(
                                    const std::string_view& bus_name) const {
    return catalogue_.GetBusInfo(bus_name);
}

const std::unordered_set<std::string_view>& RequestHandler::GetBusesByStop(
                                    const std::string_view& stop_name) const {
    return catalogue_.GetStopInfo(stop_name);
}

const std::optional<std::vector<graph::Edge<double>>> RequestHandler::GetOptimalRoute(const std::string_view stop_from, const std::string_view stop_to) const {
    return router_.FindRoute(stop_from, stop_to);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.CreateSvgDoc(catalogue_.GetAllBuses());
}
