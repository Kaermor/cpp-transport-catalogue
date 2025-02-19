#include "stat_reader.h"
#include "input_reader.h"

#include <iomanip>
#include <ostream>
#include <set>

namespace transport_catalogue{
namespace output{

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    using namespace std::literals;
    std::string_view tmp = transport_catalogue::input::Trim(request);
    std::string_view request_command = tmp.substr(0, tmp.find_first_of(' '));
    std::string_view request_id = tmp.substr(tmp.find_first_of(' ')+1);
    if(request_command == "Bus"s){
        output << "Bus "s
               << request_id;
        if(!transport_catalogue.GetBus(request_id)){
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

    else if(request_command == "Stop"s){
        output << "Stop "s
               << request_id;
        if(!transport_catalogue.GetStop(request_id)){
            output << ": not found"s
                   << std::endl;
            return;
        }
        std::set<std::string_view> buses_at_stop = transport_catalogue.GetStopInfo(request_id);
        if(buses_at_stop.empty()){
            output << ": no buses"s
                   << std::endl;
            return;
        }
        output << ": buses "s;
        for(const auto& bus : buses_at_stop){
            output << bus << " "s;
        }
        output << std::endl;
    }
}

}
}
