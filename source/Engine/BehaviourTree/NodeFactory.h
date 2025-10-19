#pragma once
#include "TreeNode.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <vector>
#include <string>

class NodeFactory {
public:
    using CreatorFunc = std::function<std::shared_ptr<TreeNode>()>;

    static NodeFactory& GetInstance();

    template<typename T>
    void RegisterNode(const std::string& typeName) {
        creators_[typeName] = []() { return std::make_shared<T>(); };
    }

    std::shared_ptr<TreeNode> CreateNode(const std::string& typeName) const;
    std::vector<std::string> GetRegisteredNodeTypes() const;

private:
    NodeFactory();
    std::unordered_map<std::string, CreatorFunc> creators_;
};

#define REGISTER_BT_NODE(type) \
    static struct type##Registrar { \
        type##Registrar() { \
            NodeFactory::GetInstance().RegisterNode<type>(#type); \
        } \
    } type##_registrar;

#define REGISTER_BT_NODE_FUNC(type) \
    NodeFactory::GetInstance().RegisterNode<type>(#type);
