#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>{
public:
    using variant::variant;
    using Value = variant;
   
    bool IsInt() const;
    bool IsPureDouble() const;
    bool IsDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    const Value& GetValue() const;

    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const;

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(std::nullptr_t, const PrintContext& ctx);
void PrintValue(const std::string& value, const PrintContext& ctx);
void PrintValue(const Array& array, const PrintContext& ctx);
void PrintValue(const Dict& map, const PrintContext& ctx);
void PrintValue(bool value, const PrintContext& ctx);

void PrintNode(const Node& node, const PrintContext& ctx);

void Print(const Document& doc, std::ostream& output);

}  // namespace json