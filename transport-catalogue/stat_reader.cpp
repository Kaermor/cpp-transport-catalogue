#include "stat_reader.h"
#include "input_reader.h"

#include <algorithm>
#include <iomanip>
#include <ostream>

namespace transport_catalogue {
namespace output {

void PrintStatBus(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request_id, std::ostream& output) {
    using namespace std::literals;
    output << "Bus "s
           << request_id;
    if (!transport_catalogue.GetBus(request_id)) {
        output << ": not found"s
               << std::endl;
        return;
    }
    transport_catalogue::BusRouteInfo bus_route_info = transport_catalogue.GetBusInfo(request_id);
    output << ": "s
           << std::to_string(bus_route_info.stops_count)
           << " stops on route, "s
           << std::to_string(bus_route_info.unique_stops)
           << " unique stops, "s
           << std::setprecision(6)
           << bus_route_info.route_length
           << " route length"s
           << std::endl;
}

void PrintStatStop(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request_id, std::ostream& output) {
    using namespace std::literals;
    output << "Stop "s
           << request_id;
    if (!transport_catalogue.GetStop(request_id)) {
        output << ": not found"s
               << std::endl;
        return;
    }
    std::unordered_set<std::string_view> set_buses_at_stop = transport_catalogue.GetStopInfo(request_id);
    if (set_buses_at_stop.empty()) {
        output << ": no buses"s
               << std::endl;
        return;
    }
    output << ": buses "s;
    std::vector<std::string_view> vec_buses_at_stop(set_buses_at_stop.begin(), set_buses_at_stop.end());
    std::sort(vec_buses_at_stop.begin(), vec_buses_at_stop.end());
    for (const auto& bus : vec_buses_at_stop) {
        output << bus << " "s;
    }
    output << std::endl;
}

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    using namespace std::literals;
    std::string_view tmp = transport_catalogue::input::Trim(request);
    std::string_view request_command = tmp.substr(0, tmp.find_first_of(' '));
    std::string_view request_id = tmp.substr(tmp.find_first_of(' ')+1);

    if (request_command == "Bus"s) {
        PrintStatBus(transport_catalogue, request_id, output);
    }
    else if (request_command == "Stop"s) {
        PrintStatStop(transport_catalogue, request_id, output);
    }
}

}
}
