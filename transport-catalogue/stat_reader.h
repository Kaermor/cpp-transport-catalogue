#pragma once

#include "transport_catalogue.h"

#include <iosfwd>
#include <string_view>

namespace transport_catalogue{
namespace output{

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);

}
}
