#include "json.h"

namespace json {

namespace {
using namespace std::literals;

Node LoadNode(std::istream& input);

std::string LoadLiteral(std::istream& input) {
    std::string str;    
    while (std::isalpha(input.peek())) {
        str.push_back(static_cast<char>(input.get()));
    }
    return str;
}

Node LoadNull(std::istream& input) {
    if (auto null_str = LoadLiteral(input); null_str == "null"sv) {
        return {};
    } else {
        throw ParsingError("Failed to parse '"s + null_str + "' as null"s);
    }
}

Node LoadBool(std::istream& input) {
    if(auto bool_str = LoadLiteral(input); bool_str == "true"sv) {
        return Node(true);
    } else if(bool_str == "false"sv) {
        return Node(false);
    } else {
        throw ParsingError("Failed to parse '"s + bool_str + "' as bool"s);
    }
}

Node LoadArray(std::istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!input) {
        throw ParsingError("Array parsing error"s);
    }

    return Node(std::move(result));
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }

    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }

    bool is_int = true;

    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return std::stoi(parsed_num);
            } catch (...) {
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadDict(std::istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':') {
                if (result.find(key) != result.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                result.emplace(std::move(key), LoadNode(input));
            } else {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        } else if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }

    if(!input) {
        throw ParsingError("Dictionary parsing error"s);
    }

    return Node(std::move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    if(!(input >> c)) {
        throw ParsingError("Unexpected EOF"s);
    }
    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 't':
            [[fallthrough]];
        case 'f':
            input.putback(c);
            return LoadBool(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        default:
            input.putback(c);
            return LoadNumber(input);
    }
}

}  // namespace

const Node::Value& Node::GetValue() const {
    return *this;
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

int Node::AsInt() const {
    using namespace std::literals;
    if (!IsInt()) {
        throw std::logic_error("Not an int"s);
    }
    return std::get<int>(*this);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Node is not a double"s);
    }
    if (IsInt()) {
        return static_cast<double>(std::get<int>(*this));
    }
    return std::get<double>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Node is not a bool"s);
    }
    return std::get<bool>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Node is not a string"s);
    }
    return std::get<std::string>(*this);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Node is not an array"s);
    }
    return std::get<Array>(*this);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Node is not a dictionary"s);
    }
    return std::get<Dict>(*this);
}

bool Node::operator==(const Node& other) const {
    return *this == other.GetValue();
}

bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void PrintContext::PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null";
}

void PrintValue(const std::string& value, const PrintContext& ctx) {
    ctx.out << '\"';
    for (const char& c : value) {
        if (c == '\n') {
            ctx.out << "\\n"sv;
            continue;
        }
        if (c == '\r') {
            ctx.out << "\\r"sv;
            continue;
        }
        if (c == '\"') {
            ctx.out << "\\"sv;
        }
        if (c == '\t') {
            ctx.out << "\\t"sv;
            continue;
        }
        if (c == '\\') {
            ctx.out << "\\"sv;
        }
        ctx.out << c;
    }
    ctx.out << '\"';
}

void PrintValue(const Array& array, const PrintContext& ctx) {
    ctx.out << '[';
    ctx.out << '\n';
    bool isFirst = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& node : array) {
        if (isFirst) {
            isFirst = false;
        }  else {
            ctx.out << ',';
            ctx.out << '\n';
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);    
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << ']';
}

void PrintValue(const Dict& map, const PrintContext& ctx) {
    ctx.out << '{';
    ctx.out << '\n';
    bool isFirst = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : map) {
        if (isFirst) {
            isFirst = false;
        }  else {
            ctx.out << ',';
            ctx.out << '\n';
        }
        inner_ctx.PrintIndent();
        PrintValue(key, ctx);
        ctx.out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << '}';
}

void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << (value ? "true" : "false");
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); }
        , node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{ output });
}

}  // namespace json