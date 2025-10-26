#include "TreeNode.h"

#include "BehaviorTree.h"
#include <random>

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

void TreeNode::SetParent(TreeNode* parent)
{
    parent_ = parent; 

    id_ = GenerateId(); //since path changed
}

json TreeNode::ToJson() const {
    json j;
    j["id"] = id_;  // Save ID
    j["name"] = name_;
    j["type"] = type_;

    // Serialize children with their IDs
    json childrenArray = json::array();
    for (const auto& child : children_) {
        childrenArray.push_back(child->ToJson());
    }
    j["children"] = childrenArray;

    return j;
}

void TreeNode::FromJson(const json& j) {
    // Load ID if present, otherwise keep the generated one
    if (j.contains("id")) {
        id_ = j["id"].get<std::string>();
    }
    name_ = j.value("name", name_);
}

json TreeNode::SaveState() const {
    json j;
    j["id"] = id_;
    j["status"] = static_cast<int>(status_);

    // Save children states by ID
    json childrenStates = json::object();
    for (const auto& child : children_) {
        childrenStates[child->GetId()] = child->SaveState();
    }
    j["children"] = childrenStates;

    return j;
}

void TreeNode::LoadState(const json& j) {
    // Only load status if the ID matches (safety check)
    std::string savedId = j.value("id", "");
    if (savedId.empty() || savedId == id_) {
        if (j.contains("status")) {
            status_ = static_cast<NodeStatus>(j["status"].get<int>());
        }
    }

    // Load children states by ID lookup
    if (j.contains("children") && !children_.empty()) {
        const auto& childrenStates = j["children"];
        if (childrenStates.is_object()) {
            for (const auto& child : children_) {
                std::string childId = child->GetId();
                if (childrenStates.contains(childId)) {
                    child->LoadState(childrenStates[childId]);
                }
                // If child ID not found in saved state, it's a new node - leave with default state
            }
        }
    }
}

void TreeNode::OnStart(BehaviorTreeContext& context)
{

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


    if (context.tree) {
        context.tree->SetLastExecutedNode(this);
        context.tree->AddActiveNode(this);
    }

    if (isTask)
    {
        context.reachedTask = true;
    }

    if (status_ != NodeStatus::Running) 
    {
        status_ = NodeStatus::Running;
        OnStart(context);

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

std::string TreeNode::GenerateId() 
{
    std::vector<std::string> pathComponents;

    // Start with node's own type and name
    pathComponents.push_back(type_);
    pathComponents.push_back(name_);

    // Walk up the parent chain to build full path
    const TreeNode* current = this;
    while (current->parent_ != nullptr) {
        // Find this node's index in parent's children
        int childIndex = current->parent_->FindChildIndex(const_cast<TreeNode*>(current));
        if (childIndex != -1) {
            pathComponents.push_back(std::to_string(childIndex));
        }
        // Add parent's type and name
        pathComponents.push_back(current->parent_->GetType());
        pathComponents.push_back(current->parent_->GetName());

        current = current->parent_;
    }

    // Reverse so we start from root
    std::reverse(pathComponents.begin(), pathComponents.end());

    // Create a hash of the path
    std::string fullPath;
    for (const auto& component : pathComponents) {
        fullPath += component + "/";
    }

    // Generate hash
    std::size_t hash = std::hash<std::string>{}(fullPath);
    std::stringstream ss;
    ss << "node_" << std::hex << hash;

    return ss.str();
}
