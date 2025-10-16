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

void TreeNode::RemoveChild(TreeNode* child)
{
    auto it = std::find_if(children_.begin(), children_.end(),
        [child](const std::shared_ptr<TreeNode>& c) { return c.get() == child; });
    if (it != children_.end()) {
        children_.erase(it);
    }
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
    return j;
}

void TreeNode::LoadState(const json& j) {
    if (j.contains("status")) {
        status_ = static_cast<NodeStatus>(j["status"].get<int>());
    }
}

void TreeNode::OnStart(BehaviorTreeContext& context)
{
    if (context.tree) {
        context.tree->SetLastExecutedNode(this);
        context.tree->AddActiveNode(this);
    }
}

NodeStatus TreeNode::Tick(BehaviorTreeContext& context) {


    if (status_ == NodeStatus::Idle) {
        OnStart(context);
        status_ = NodeStatus::Running;
    }

    NodeStatus result = Execute(context);

    // Remove from active nodes if completed
    if (context.tree && result != NodeStatus::Running) {
        context.tree->RemoveActiveNode(this);
    }

    if (result != NodeStatus::Running) {
        OnStop(context);
        status_ = result;
    }
    else {
        status_ = NodeStatus::Running;
    }
    return status_;
}