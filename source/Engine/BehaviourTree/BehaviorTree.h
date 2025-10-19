#pragma once
#include "TreeNode.h"
#include "Blackboard.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "../json.hpp"
#include <deque>

using nlohmann::json;

class BehaviorTree 
{

private:

    TreeNode* lastExecutedNode_ = nullptr;
    std::deque<TreeNode*> executionHistory_; // Track recent execution history

public:
    BehaviorTree();
    ~BehaviorTree();

    // Execution
    void Start();
    void Stop();
    void Update(float deltaTime);
    void Reset();

    // Tree structure
    void SetRoot(std::shared_ptr<TreeNode> root);
    std::shared_ptr<TreeNode> GetRoot() const { return root_; }

    // Blackboard access
    Blackboard& GetBlackboard() { return blackboard_; }
    const Blackboard& GetBlackboard() const { return blackboard_; }

    // State management
    bool IsRunning() const { return isRunning_; }
    NodeStatus GetLastRootStatus() const { return lastRootStatus_; }

    // Abort system
    void RequestAbort(TreeNode* node);
    void ProcessAborts();

    // Active nodes tracking (for abort system)
    void AddActiveNode(TreeNode* node);
    void RemoveActiveNode(TreeNode* node);
    const std::vector<TreeNode*>& GetActiveNodes() const { return activeNodes_; }

    // JSON Serialization
    json ToJson() const;
    bool FromJson(const json& j);

    // State serialization
    json SaveState() const;
    bool LoadState(const json& j);

    // File operations
    bool SaveToFile(const std::string& filename) const;
    bool LoadFromFile(const std::string& filename);
    bool SaveStateToFile(const std::string& filename) const;
    bool LoadStateFromFile(const std::string& filename);

    // Execution tracking
    TreeNode* GetLastExecutedNode() const { return lastExecutedNode_; }

    void SetLastExecutedNode(TreeNode* node) 
    {
        lastExecutedNode_ = node;
    }

    const std::deque<TreeNode*>& GetExecutionHistory() const { return executionHistory_; }

    static void RegisterTypes();

    void* Owner = nullptr;

private:
    std::shared_ptr<TreeNode> CreateNodeFromJson(const json& j);
    void BuildTreeFromJson(std::shared_ptr<TreeNode> parent, const json& j);
    void RegisterDecoratorObservers(std::shared_ptr<TreeNode> node);
    void UnregisterDecoratorObservers(std::shared_ptr<TreeNode> node);

    std::shared_ptr<TreeNode> root_;
    Blackboard blackboard_;
    BehaviorTreeContext context_;
    bool isRunning_ = false;
    NodeStatus lastRootStatus_ = NodeStatus::Idle;

    // Abort system
    std::vector<TreeNode*> activeNodes_;
    std::vector<TreeNode*> pendingAborts_;

    // Observer management - now using ID-based tracking
    std::unordered_map<TreeNode*, std::vector<std::pair<std::string, Blackboard::ObserverID>>> decoratorObservers_;
};