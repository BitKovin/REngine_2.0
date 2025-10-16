#pragma once
#include "BehaviorTreeTypes.h"
#include <memory>
#include <vector>
#include <string>
#include <json.hpp>

using nlohmann::json;

// Forward declarations
class BehaviorTree;

class TreeNode {
public:
    explicit TreeNode(const std::string& name, const std::string& type);
    virtual ~TreeNode() = default;

    // Core functionality
    virtual NodeStatus Execute(BehaviorTreeContext& context) = 0;
    virtual void OnStart(BehaviorTreeContext& context);
    virtual void OnStop(BehaviorTreeContext& context) {}
    virtual void OnReset() { status_ = NodeStatus::Idle; }
    virtual void Abort(BehaviorTreeContext& context) {
        OnStop(context);
        status_ = NodeStatus::Idle;
    }

    // Hierarchy
    void AddChild(std::shared_ptr<TreeNode> child);
    const std::vector<std::shared_ptr<TreeNode>>& GetChildren() const { return children_; }

    void RemoveChild(TreeNode* child);

    // Properties
    const std::string& GetName() const { return name_; }
    const std::string& GetType() const { return type_; }
    NodeStatus GetStatus() const { return status_; }
    void SetStatus(NodeStatus status) { status_ = status; }

    // Tree structure
    void SetParent(TreeNode* parent) { parent_ = parent; }
    TreeNode* GetParent() const { return parent_; }

    // JSON Serialization
    virtual json ToJson() const;
    virtual void FromJson(const json& j);

    // State serialization
    virtual json SaveState() const;
    virtual void LoadState(const json& j);

    // Tick wrapper managing lifecycle. Non-virtual; calls virtual Execute/OnStart/OnStop.
    NodeStatus Tick(BehaviorTreeContext& context);

protected:
    std::string name_;
    std::string type_;
    NodeStatus status_ = NodeStatus::Idle;
    std::vector<std::shared_ptr<TreeNode>> children_;
    TreeNode* parent_ = nullptr;
};