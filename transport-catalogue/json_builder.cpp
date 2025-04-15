#include "json_builder.h"

using namespace std::literals;

namespace json {

    Builder::Builder() {
        Node* root_ptr = &root_;
        nodes_stack_.emplace_back(root_ptr);
    }

    Builder::DictValueContext Builder::Key(std::string key) {
        Node::Value& host_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(host_value)) {
            throw std::logic_error("Key() called outside of a dictionary context"s);
        }

        nodes_stack_.emplace_back(&std::get<Dict>(host_value)[std::move(key)]);
        return BaseContext{*this};
    }

    Builder::BaseContext Builder::Value(Node::Value value) {
        AddObject(std::move(value), true);
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        AddObject(Dict{}, false);
        return BaseContext{*this};
    }

    Builder::BaseContext Builder::EndDict() {
        if (!std::holds_alternative<Dict>(GetCurrentValue())) {
            throw std::logic_error("EndDict() called without a matching StartDict()"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        AddObject(Array{}, false);
        return BaseContext{*this};
    }

    Builder::BaseContext Builder::EndArray() {
        if (!std::holds_alternative<Array>(GetCurrentValue())) {
            throw std::logic_error("EndArray() called without a matching StartArray()"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Build() called before completing the JSON structure"s);
        }
        return std::move(root_);
    }

    Node Builder::BaseContext::Build() {
        return builder_.Build();
    }

    Builder::DictValueContext Builder::BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::BaseContext Builder::BaseContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext Builder::BaseContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::BaseContext Builder::BaseContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value) {
        return BaseContext::Value(std::move(value));
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return BaseContext::Value(std::move(value));
    }

    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON"s);
        }
        return nodes_stack_.back()->GetValue();
    }
    
    const Node::Value& Builder::GetCurrentValue() const {
        return const_cast<Builder*>(this)->GetCurrentValue();
    }
    
    void Builder::AssertNewObjectContext() const {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
            throw std::logic_error("New object in wrong context"s);
        }
    }
    
    void Builder::AddObject(Node::Value value, bool one_shot) {
        Node::Value& host_value = GetCurrentValue();
        if (std::holds_alternative<Array>(host_value)) {
            Node& node
                = std::get<Array>(host_value).emplace_back(std::move(value));
            if (!one_shot) {
                nodes_stack_.push_back(&node);
            }
        } else {
            AssertNewObjectContext();
            host_value = std::move(value);
            if (one_shot) {
                nodes_stack_.pop_back();
            }
        }
    }

}  // namespace json