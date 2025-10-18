#include "BehaviorTree.h"
#include "NodeFactory.h"
#include "DecoratorNodes.h"
#include <fstream>
#include <algorithm>

BehaviorTree::BehaviorTree() {
    context_.blackboard = &blackboard_;
    context_.tree = this;
}

BehaviorTree::~BehaviorTree() {
    // Unregister all observers
    UnregisterDecoratorObservers(root_);
}

void BehaviorTree::Start() {
    if (root_) {
        isRunning_ = true;
        root_->OnReset();
        activeNodes_.clear();
        pendingAborts_.clear();
        lastExecutedNode_ = nullptr; // Reset last executed

        // Register decorator observers
        RegisterDecoratorObservers(root_);
    }
}

void BehaviorTree::Stop() {
    isRunning_ = false;
    if (root_) {
        root_->OnStop(context_);
        // Unregister decorator observers
        UnregisterDecoratorObservers(root_);
    }
    activeNodes_.clear();
    pendingAborts_.clear();
    lastExecutedNode_ = nullptr; // Reset last executed
}

void BehaviorTree::Update(float deltaTime) {
    if (!isRunning_ || !root_) return;

    context_.deltaTime = deltaTime;
    context_.reachedTask = false;

    // Process any pending aborts first
    ProcessAborts();

    // Execute the tree
    NodeStatus status = root_->TickNode(context_);
    lastRootStatus_ = status;

    // Do not auto-reset on Success/Failure; leave tree state stable until explicitly Reset/Start
}

void BehaviorTree::Reset() {
    if (root_) {
        root_->OnReset();
    }
    blackboard_.Clear();
    activeNodes_.clear();
    pendingAborts_.clear();
    lastExecutedNode_ = nullptr; // Reset last executed
}

void BehaviorTree::SetRoot(std::shared_ptr<TreeNode> root) {
    // Unregister old observers
    UnregisterDecoratorObservers(root_);

    root_ = root;

    // Register new observers
    if (root_) {
        RegisterDecoratorObservers(root_);
    }
}

void BehaviorTree::RequestAbort(TreeNode* node) {
    if (node && std::find(pendingAborts_.begin(), pendingAborts_.end(), node) == pendingAborts_.end()) {
        pendingAborts_.push_back(node);
    }
}

void BehaviorTree::ProcessAborts() {
    for (TreeNode* node : pendingAborts_) {
        node->Abort(context_);

        // Remove from active nodes
        auto it = std::find(activeNodes_.begin(), activeNodes_.end(), node);
        if (it != activeNodes_.end()) {
            activeNodes_.erase(it);
        }
    }
    pendingAborts_.clear();
}

void BehaviorTree::AddActiveNode(TreeNode* node) {
    if (node && std::find(activeNodes_.begin(), activeNodes_.end(), node) == activeNodes_.end()) {
        activeNodes_.push_back(node);
    }
}

void BehaviorTree::RemoveActiveNode(TreeNode* node) {
    auto it = std::find(activeNodes_.begin(), activeNodes_.end(), node);
    if (it != activeNodes_.end()) {
        activeNodes_.erase(it);
    }
}

void BehaviorTree::RegisterDecoratorObservers(std::shared_ptr<TreeNode> node) {
    if (!node) return;

    // Check if this is a decorator with observer keys
    DecoratorNode* decorator = dynamic_cast<DecoratorNode*>(node.get());
    if (decorator && decorator->GetAbortMode() != FlowAbortMode::None) {
        std::vector<std::pair<std::string, Blackboard::ObserverID>> observerIds;

        for (const auto& key : decorator->GetObserverKeys()) {
            auto callback = [this, decorator](const std::string& changedKey, const Blackboard::Value& value) {
                // Check if the decorator condition has changed
                bool currentCondition = decorator->CheckCondition(this->context_);

                // For now, we'll always abort if the observed key changes
                // In a more sophisticated system, we'd check the specific condition
                if (decorator->GetAbortMode() == FlowAbortMode::Self ||
                    decorator->GetAbortMode() == FlowAbortMode::Both) {
                    RequestAbort(decorator);
                }
                };

            Blackboard::ObserverID id = blackboard_.AddObserver(key, callback);
            observerIds.emplace_back(key, id);
        }

        decoratorObservers_[node.get()] = std::move(observerIds);
    }

    // Recursively register for children
    for (const auto& child : node->GetChildren()) {
        RegisterDecoratorObservers(child);
    }
}

void BehaviorTree::UnregisterDecoratorObservers(std::shared_ptr<TreeNode> node) {
    if (!node) return;

    // Unregister observers for this specific node
    auto it = decoratorObservers_.find(node.get());
    if (it != decoratorObservers_.end()) {
        for (const auto& [key, observerId] : it->second) {
            blackboard_.RemoveObserver(key, observerId);
        }
        decoratorObservers_.erase(it);
    }

    // Recursively unregister for children
    for (const auto& child : node->GetChildren()) {
        UnregisterDecoratorObservers(child);
    }
}

json BehaviorTree::ToJson() const {
    json j;

    if (root_) {
        j["root"] = root_->ToJson();
    }

    j["blackboard"] = blackboard_.ToJson();
    j["isRunning"] = isRunning_;

    return j;
}

bool BehaviorTree::FromJson(const json& j) {
    Stop();
    Reset();

    if (j.contains("root")) {
        auto rootJson = j["root"];
        std::string rootType = rootJson.value("type", "");
        if (!rootType.empty()) {
            root_ = NodeFactory::GetInstance().CreateNode(rootType);
            if (root_) {
                root_->FromJson(rootJson);
                BuildTreeFromJson(root_, rootJson);
            }
        }
    }

    if (j.contains("blackboard")) {
        blackboard_.FromJson(j["blackboard"]);
    }

    isRunning_ = j.value("isRunning", false);

    // Register observers for the new tree
    if (root_) {
        RegisterDecoratorObservers(root_);
    }

    return root_ != nullptr;
}

json BehaviorTree::SaveState() const {
    json j;

    if (root_) {
        j["root"] = root_->SaveState();
    }

    j["blackboard"] = blackboard_.ToJson();
    j["isRunning"] = isRunning_;

    return j;
}

bool BehaviorTree::LoadState(const json& j) {
    if (j.contains("root") && root_) {
        root_->LoadState(j["root"]);
    }

    if (j.contains("blackboard")) {
        blackboard_.FromJson(j["blackboard"]);
    }

    isRunning_ = j.value("isRunning", isRunning_);

    return true;
}

bool BehaviorTree::SaveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << ToJson().dump(4);
    return true;
}

bool BehaviorTree::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    json j;
    file >> j;
    return FromJson(j);
}

bool BehaviorTree::SaveStateToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << SaveState().dump(4);
    return true;
}

bool BehaviorTree::LoadStateFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    json j;
    file >> j;
    return LoadState(j);
}

std::shared_ptr<TreeNode> BehaviorTree::CreateNodeFromJson(const json& j) {
    std::string nodeType = j.value("type", "");
    if (nodeType.empty()) return nullptr;

    return NodeFactory::GetInstance().CreateNode(nodeType);
}

void BehaviorTree::BuildTreeFromJson(std::shared_ptr<TreeNode> parent, const json& j) {
    if (!j.contains("children")) return;

    for (const auto& childJson : j["children"]) {
        auto child = CreateNodeFromJson(childJson);
        if (child) {
            child->FromJson(childJson);
            parent->AddChild(child);
            BuildTreeFromJson(child, childJson);
        }
    }
}