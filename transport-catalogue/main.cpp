#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    TransportCatalogue catalogue;
    json_reader::JsonReader json_doc(cin);
    json_doc.FillTransportCatalogue(catalogue);
    const auto& rend_settings = json_doc.GetRenderSettings();
    const auto& map_renderer = json_doc.SetRenderSettings(rend_settings);
    const auto& routing_settings = json_doc.SetRoutingSettings(json_doc.GetRoutingSettings());
    transport_router::TransportRouter router(catalogue, routing_settings);
    RequestHandler request_handler(catalogue
                                , map_renderer
                                , router);
    json_doc.ProcessRequests(json_doc.GetStatRequests(), request_handler);
    
    return 0;
}