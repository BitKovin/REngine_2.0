#include "TreeNode.h"

#include "BehaviorTree.h"

TreeNode::TreeNode(const std::string& name, const std::string& type)
    : name_(name), type_(type) {
}

void TreeNode::AddChild(std::shared_ptr<TreeNode> child) {
    if (child) {
        child->SetParent(this);
        children_.push_back(child);
    }
}

bool TreeNode::RemoveChild(TreeNode* child) {
    auto it = std::find_if(children_.begin(), children_.end(),
        [child](const std::shared_ptr<TreeNode>& c) { return c.get() == child; });
    if (it != children_.end()) {
        (*it)->SetParent(nullptr);
        children_.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<TreeNode> TreeNode::ExtractChild(TreeNode* child) {
    auto it = std::find_if(children_.begin(), children_.end(),
        [child](const std::shared_ptr<TreeNode>& c) { return c.get() == child; });
    if (it != children_.end()) {
        auto extracted = *it;
        extracted->SetParent(nullptr);
        children_.erase(it);
        return extracted;
    }
    return nullptr;
}

void TreeNode::InsertChild(size_t index, std::shared_ptr<TreeNode> child) {
    if (index > children_.size()) {
        index = children_.size();
    }
    child->SetParent(this);
    children_.insert(children_.begin() + index, child);
}

int TreeNode::FindChildIndex(TreeNode* child) const {
    auto it = std::find_if(children_.begin(), children_.end(),
        [child](const std::shared_ptr<TreeNode>& c) { return c.get() == child; });
    return (it != children_.end()) ? static_cast<int>(std::distance(children_.begin(), it)) : -1;
}

json TreeNode::ToJson() const {
    json j;
    j["name"] = name_;
    j["type"] = type_;

    // Serialize children
    json childrenArray = json::array();
    for (const auto& child : children_) {
        childrenArray.push_back(child->ToJson());
    }
    j["children"] = childrenArray;

    return j;
}

void TreeNode::FromJson(const json& j) {
    name_ = j.value("name", name_);
    // Note: type_ is not set from JSON as it's fixed per node class
}

json TreeNode::SaveState() const {
    json j;
    j["status"] = static_cast<int>(status_);

    // Only save children states if we have children
    // Don't save the actual children structure, just their runtime state
    if (!children_.empty()) {
        json childrenStates = json::array();
        for (const auto& child : children_) {
            // Save each child's state
            childrenStates.push_back(child->SaveState());
        }
        j["children"] = childrenStates;
    }

    return j;
}

void TreeNode::LoadState(const json& j) {
    if (j.contains("status")) {
        status_ = static_cast<NodeStatus>(j["status"].get<int>());
    }

    // Load children states - only if we have matching children
    if (j.contains("children") && !children_.empty()) {
        const auto& childrenStates = j["children"];
        if (childrenStates.is_array()) {
            // Only load states for existing children (safe if tree structure changed)
            size_t minSize = std::min(childrenStates.size(), children_.size());
            for (size_t i = 0; i < minSize; ++i) {
                children_[i]->LoadState(childrenStates[i]);
            }
        }
    }
}

void TreeNode::OnStart(BehaviorTreeContext& context)
{
    if (context.tree) {
        context.tree->SetLastExecutedNode(this);
        context.tree->AddActiveNode(this);
    }
}

void TreeNode::OnStop(BehaviorTreeContext& context)
{

    SetStatus(NodeStatus::Idle);

    for (auto child : children_)
    {
        child->OnStop(context);
    }
}

NodeStatus TreeNode::TickNode(BehaviorTreeContext& context) {


    if (isTask)
    {
        context.reachedTask = true;
    }

    if (status_ != NodeStatus::Running) {
        OnStart(context);
        status_ = NodeStatus::Running;
    }

    NodeStatus result = Execute(context);

    // Remove from active nodes if completed
    if (context.tree && result != NodeStatus::Running) {
        context.tree->RemoveActiveNode(this);
    }

    if (result != NodeStatus::Running) {
        status_ = result;
    }
    else {
        status_ = NodeStatus::Running;
    }
    return status_;
}