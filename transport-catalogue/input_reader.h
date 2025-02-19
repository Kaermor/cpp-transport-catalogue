#pragma once

#include "transport_catalogue.h"

#include <string>
#include <string_view>
#include <vector>

namespace transport_catalogue{
namespace input{

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

std::string_view Trim(std::string_view string);

std::vector<std::string_view> Split(std::string_view string, char delim);

class InputReader {
public:
    void ParseLine(std::string_view line);

    void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> commands_;
};

}
}
