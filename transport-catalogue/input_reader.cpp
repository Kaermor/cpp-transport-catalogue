#include "input_reader.h"

#include <cassert>
#include <iterator>

namespace transport_catalogue {
namespace input {

Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

std::unordered_map<std::string, uint32_t> ParseStopDistance(std::string_view line) {
    std::unordered_map<std::string, uint32_t> result;
    auto not_space = line.find_first_not_of(' ');
    auto comma_pos = line.find(',', not_space + 1);
    auto space_pos = line.find_first_of(' ', not_space);
    bool is_first = true;
    while (not_space != line.npos) {
        if(!is_first) {
            not_space = line.find_first_not_of(' ', comma_pos + 1);
            comma_pos = line.find(',', not_space + 1);
            space_pos = line.find_first_of(' ', not_space);
        }
        uint32_t distance = std::stoi(std::string(line.substr(not_space, space_pos - 1)));
            not_space = line.find_first_not_of(' ', space_pos);
            space_pos = line.find_first_of(' ', not_space);
            not_space = line.find_first_not_of(' ', space_pos);
            result.insert({std::string(line.substr(not_space, comma_pos - not_space)), distance});
            is_first = false;
            not_space = comma_pos;
    }
    return result;
}

std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

StopDescription ParseStopDescription(std::string_view line) {
    StopDescription result;
    auto comma_pos = line.find(',');
    if (comma_pos == line.npos) {
        return result;
    }

    auto comma2_pos = line.find(',', comma_pos + 1);
    result.coordinates = ParseCoordinates(line.substr(0, comma2_pos));
    if (comma2_pos == line.npos) {
        return result;
    }

    result.stop2stop_distances = ParseStopDistance(line.substr(comma2_pos + 1));

    return result;
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] transport_catalogue::TransportCatalogue& catalogue) const {
    using namespace std::literals;
    std::vector<StopDescription> all_stops_description;
    for (const auto& c : commands_) {
        if (c.command == "Bus"s) {
            continue;
        }
        StopDescription stop_description = ParseStopDescription(c.description);
        stop_description.stop_name = c.id;
        catalogue.AddStop(stop_description.stop_name, stop_description.coordinates);
        all_stops_description.push_back(stop_description);
    }

    for (const auto& stop_desc : all_stops_description) {
        for (const auto& [stop_to, distance] : stop_desc.stop2stop_distances) {
            catalogue.AddStop2StopDistance(stop_desc.stop_name, stop_to, distance);
        }
    }

    for (const auto& c : commands_) {
        if (c.command == "Stop"s) {
            continue;
        }
        catalogue.AddBus(c.id, ParseRoute(c.description));
    }
}

} // namespace input
} // namespace transport_catalogue
