#pragma once

#include "json.h"

namespace json {

class Builder {
private:
    class BaseContext;
    class DictItemContext;
    class DictValueContext;
    class ArrayItemContext;

public:
    Builder();

    DictValueContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    BaseContext EndDict();
    ArrayItemContext StartArray();
    BaseContext EndArray();
    Node Build();


private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;
    
    void AssertNewObjectContext() const;
    void AddObject(Node::Value value, bool one_shot);

    class BaseContext {
    public:
        BaseContext(Builder& builder) : builder_(builder) {}
        Node Build();
        DictValueContext Key(std::string key);
        BaseContext Value(Node::Value value);
        DictItemContext StartDict();
        BaseContext EndDict();
        ArrayItemContext StartArray();
        BaseContext EndArray();

    private:
        Builder& builder_;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base) : BaseContext(base) {};
        Node Build() = delete;
        DictItemContext Value(Node::Value value) = delete;
        DictItemContext StartDict() = delete;
        DictItemContext StartArray() = delete;
        DictItemContext EndArray() = delete;

    };

    class DictValueContext : public BaseContext {
        public:
            DictValueContext(BaseContext base) : BaseContext(base) {}
            DictItemContext Value(Node::Value value);
            Node Build() = delete;
            DictValueContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base) : BaseContext(base) {};
        ArrayItemContext Value(Node::Value value);
        Node Build() = delete;
        DictItemContext Key(std::string key) = delete;
        DictItemContext EndDict() = delete;
    };
};

}  // namespace json