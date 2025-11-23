#include "BehaviorTree.h"
#include "NodeFactory.h"
#include "DecoratorNodes.h"
#include <fstream>
#include <algorithm>
#include "../FileSystem/FileSystem.h"

BehaviorTree::BehaviorTree() {
    context_.blackboard = &blackboard_;
    context_.tree = this;
}

BehaviorTree::~BehaviorTree() {
}

void BehaviorTree::Start() {
    if (root_) {
        isRunning_ = true;
        root_->OnReset();
        activeNodes_.clear();
        pendingAborts_.clear();
        lastExecutedNode_ = nullptr; // Reset last executed

    }
}

void BehaviorTree::Stop() {
    isRunning_ = false;
    if (root_) {
        root_->OnStop(context_);
    }
    activeNodes_.clear();
    pendingAborts_.clear();
    lastExecutedNode_ = nullptr; // Reset last executed
}

void BehaviorTree::Update(float deltaTime) {
    if (!isRunning_ || !root_) return;

    context_.deltaTime = deltaTime;
    context_.reachedTask = false;
    context_.hasToFinishDecorator = false;
    context_.owner = Owner;

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

    root_ = root;

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

bool BehaviorTree::LoadFromFile(const std::string& filename) 
{
    
    std::string s = FileSystemEngine::ReadFile(filename);

    if (s.empty())
        return false;

    json j = json::parse(s);

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

#include "CompositeNodes.h"
#include "TaskNodes.h"

void BehaviorTree::RegisterTypes()
{

    // Register nodes
    REGISTER_BT_NODE_FUNC(SequenceNode);
    REGISTER_BT_NODE_FUNC(MultiExecSequenceNode);
    REGISTER_BT_NODE_FUNC(SelectorNode);
    REGISTER_BT_NODE_FUNC(WaitNode);
    REGISTER_BT_NODE_FUNC(PrintMessageNode);

    REGISTER_BT_NODE_FUNC(RepeatDecorator);
    REGISTER_BT_NODE_FUNC(ConditionalDecorator);

}