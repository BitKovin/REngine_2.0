#include "CompositeNodes.h"

SequenceNode::SequenceNode() : CompositeNode("Sequence", "SequenceNode") {}

void SequenceNode::OnStart(BehaviorTreeContext& context) 
{

    currentChildIndex_ = 0;
}

NodeStatus SequenceNode::Execute(BehaviorTreeContext& context) {
    // Start from current child index (for resuming after Running)
    for (size_t i = currentChildIndex_; i < children_.size(); ++i) {
        auto& child = children_[i];

        bool startingNode = child->GetStatus() == NodeStatus::Idle;

        NodeStatus childStatus = child->Tick(context);


        if (childStatus == NodeStatus::Running || startingNode) {
            currentChildIndex_ = i;
            return NodeStatus::Running;
        }
        else if (childStatus == NodeStatus::Failure) {
            // UE: on failure, sequence fails immediately; next tick should start from first child again
            currentChildIndex_ = 0;
            return NodeStatus::Failure;
        }
        // Success: advance to next child; next child will OnStart on first Tick
    }

    // All children succeeded. Keep returning Success until external flow changes (e.g., aborts or reset).
    // Next evaluation should begin from first child again.
    currentChildIndex_ = 0;
    return NodeStatus::Success;
}

void SequenceNode::OnReset() {
    TreeNode::OnReset();
    currentChildIndex_ = 0;
    for (auto& child : children_) {
        child->OnReset();
    }
}

void SequenceNode::Abort(BehaviorTreeContext& context) {
    // Abort current running child
    if (currentChildIndex_ < children_.size()) {
        children_[currentChildIndex_]->Abort(context);
    }
    TreeNode::Abort(context);
}

json SequenceNode::SaveState() const {
    auto j = TreeNode::SaveState();
    j["currentChildIndex"] = currentChildIndex_;
    return j;
}

void SequenceNode::LoadState(const json& j) {
    TreeNode::LoadState(j);
    if (j.contains("currentChildIndex")) {
        currentChildIndex_ = j["currentChildIndex"].get<size_t>();
    }
}

SelectorNode::SelectorNode() : CompositeNode("Selector", "SelectorNode") {}

void SelectorNode::OnStart(BehaviorTreeContext& context) {
    currentChildIndex_ = 0;
}

NodeStatus SelectorNode::Execute(BehaviorTreeContext& context) {
    for (size_t i = currentChildIndex_; i < children_.size(); ++i) {
        auto& child = children_[i];
        NodeStatus childStatus = child->Tick(context);

        if (childStatus == NodeStatus::Running) {
            currentChildIndex_ = i;
            return NodeStatus::Running;
        }
        else if (childStatus == NodeStatus::Success) {
            // UE: on success, selector succeeds immediately; next tick should start from first child again
            currentChildIndex_ = 0;
            return NodeStatus::Success;
        }
    }

    // All children failed
    currentChildIndex_ = 0;
    return NodeStatus::Failure;
}

void SelectorNode::OnReset() {
    TreeNode::OnReset();
    currentChildIndex_ = 0;
    for (auto& child : children_) {
        child->OnReset();
    }
}

void SelectorNode::Abort(BehaviorTreeContext& context) {
    // Abort current running child
    if (currentChildIndex_ < children_.size()) {
        children_[currentChildIndex_]->Abort(context);
    }
    TreeNode::Abort(context);
}

json SelectorNode::SaveState() const {
    auto j = TreeNode::SaveState();
    j["currentChildIndex"] = currentChildIndex_;
    return j;
}

void SelectorNode::LoadState(const json& j) {
    TreeNode::LoadState(j);
    if (j.contains("currentChildIndex")) {
        currentChildIndex_ = j["currentChildIndex"].get<size_t>();
    }
}