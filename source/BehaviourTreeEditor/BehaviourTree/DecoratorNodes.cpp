#include "DecoratorNodes.h"
#include "BehaviorTree.h"

DecoratorNode::DecoratorNode(const std::string& name, const std::string& type)
    : TreeNode(name, type) {
}

void DecoratorNode::SetChild(std::shared_ptr<TreeNode> child) {
    if (!children_.empty()) {
        children_.clear();
    }
    AddChild(child);
}

std::shared_ptr<TreeNode> DecoratorNode::GetChild() const {
    return children_.empty() ? nullptr : children_.front();
}

InverterDecorator::InverterDecorator() : DecoratorNode("Inverter", "InverterDecorator") {}

NodeStatus InverterDecorator::Execute(BehaviorTreeContext& context) {
    if (children_.empty()) return NodeStatus::Failure;

    NodeStatus childStatus = children_[0]->Execute(context);

    switch (childStatus) {
    case NodeStatus::Success: return NodeStatus::Failure;
    case NodeStatus::Failure: return NodeStatus::Success;
    case NodeStatus::Running: return NodeStatus::Running;
    default: return NodeStatus::Failure;
    }
}

RepeatDecorator::RepeatDecorator()
    : DecoratorNode("Repeat", "RepeatDecorator") {
    repeatCount_.SetConstantValue(-1); // Infinite by default
}

void RepeatDecorator::OnStart(BehaviorTreeContext& context) {
    currentCount_ = 0;
}

NodeStatus RepeatDecorator::Execute(BehaviorTreeContext& context) {
    if (children_.empty()) return NodeStatus::Failure;

    // Resolve repeat count from BTVariable
    int resolvedRepeatCount = context.blackboard->ResolveBTVariable(repeatCount_, -1);

    while (resolvedRepeatCount < 0 || currentCount_ < resolvedRepeatCount) {
        NodeStatus childStatus = children_[0]->Execute(context);

        if (childStatus == NodeStatus::Running) {
            return NodeStatus::Running;
        }
        else if (childStatus == NodeStatus::Failure) {
            return NodeStatus::Failure;
        }

        currentCount_++;

        if (resolvedRepeatCount > 0 && currentCount_ >= resolvedRepeatCount) {
            return NodeStatus::Success;
        }

        // Reset child for next iteration
        if (childStatus == NodeStatus::Success) {
            children_[0]->OnReset();
        }
    }

    return NodeStatus::Success;
}

json RepeatDecorator::ToJson() const {
    auto j = DecoratorNode::ToJson();
    j["repeatCount"] = repeatCount_.ToJson();
    return j;
}

void RepeatDecorator::FromJson(const json& j) {
    DecoratorNode::FromJson(j);
    if (j.contains("repeatCount")) {
        repeatCount_.FromJson(j["repeatCount"]);
    }
}

json RepeatDecorator::SaveState() const {
    auto j = DecoratorNode::SaveState();
    j["currentCount"] = currentCount_;
    return j;
}

void RepeatDecorator::LoadState(const json& j) {
    DecoratorNode::LoadState(j);
    if (j.contains("currentCount")) {
        currentCount_ = j["currentCount"].get<int>();
    }
}

SucceederDecorator::SucceederDecorator() : DecoratorNode("Succeeder", "SucceederDecorator") {}

NodeStatus SucceederDecorator::Execute(BehaviorTreeContext& context) {
    if (children_.empty()) return NodeStatus::Success;

    NodeStatus childStatus = children_[0]->Execute(context);

    if (childStatus == NodeStatus::Running) {
        return NodeStatus::Running;
    }

    return NodeStatus::Success;
}

FailerDecorator::FailerDecorator() : DecoratorNode("Failer", "FailerDecorator") {}

NodeStatus FailerDecorator::Execute(BehaviorTreeContext& context) {
    if (children_.empty()) return NodeStatus::Failure;

    NodeStatus childStatus = children_[0]->Execute(context);

    if (childStatus == NodeStatus::Running) {
        return NodeStatus::Running;
    }

    return NodeStatus::Failure;
}

ConditionalDecorator::ConditionalDecorator()
    : DecoratorNode("Conditional", "ConditionalDecorator") {
    condition_.SetConstantValue(true);
    SetAbortMode(FlowAbortMode::Self);
}

NodeStatus ConditionalDecorator::Execute(BehaviorTreeContext& context) {
    if (children_.empty()) return NodeStatus::Failure;

    bool condition = CheckCondition(context);
    if (!condition) {
        return NodeStatus::Failure;
    }

    return children_[0]->Execute(context);
}

bool ConditionalDecorator::CheckCondition(BehaviorTreeContext& context) const {
    return context.blackboard->ResolveBTVariable(condition_, true);
}

json ConditionalDecorator::ToJson() const {
    auto j = DecoratorNode::ToJson();
    j["condition"] = condition_.ToJson();
    j["abortMode"] = static_cast<int>(abortMode_);

    json observerArray = json::array();
    for (const auto& key : observerKeys_) {
        observerArray.push_back(key);
    }
    j["observerKeys"] = observerArray;

    return j;
}

void ConditionalDecorator::FromJson(const json& j) {
    DecoratorNode::FromJson(j);
    if (j.contains("condition")) {
        condition_.FromJson(j["condition"]);
    }
    if (j.contains("abortMode")) {
        abortMode_ = static_cast<FlowAbortMode>(j["abortMode"].get<int>());
    }
    if (j.contains("observerKeys")) {
        observerKeys_.clear();
        for (const auto& key : j["observerKeys"]) {
            observerKeys_.push_back(key.get<std::string>());
        }
    }
}