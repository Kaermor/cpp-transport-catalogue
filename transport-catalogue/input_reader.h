#pragma once

#include "transport_catalogue.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace transport_catalogue {
namespace input {

struct CommandDescription {
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;
    std::string id;
    std::string description;
};

struct StopDescription {
    std::string stop_name;
    Coordinates coordinates;
    std::unordered_map<std::string, uint32_t> stop2stop_distances;
};

std::string_view Trim(std::string_view string);

std::vector<std::string_view> Split(std::string_view string, char delim);

class InputReader {
public:
    void ParseLine(std::string_view line);

    void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> commands_;
};

} // namespace input
} // namespace transport_catalogue
