#include "TaskNodes.h"
#include "Blackboard.h"
#include <iostream>

WaitNode::WaitNode() : TreeNode("Wait", "WaitNode") {
    waitTime_.SetConstantValue(1.0f); // Default wait time
}

void WaitNode::OnStart(BehaviorTreeContext& context) 
{

    TreeNode::OnStart(context);

    elapsedTime_ = 0.0f;
}

NodeStatus WaitNode::Execute(BehaviorTreeContext& context) {
    // Resolve wait time from BTVariable (could be constant or blackboard value)
    float resolvedWaitTime = context.blackboard->ResolveBTVariable(waitTime_, 1.0f);

    elapsedTime_ += context.deltaTime;

    if (elapsedTime_ >= resolvedWaitTime) {
        return NodeStatus::Success;
    }

    return NodeStatus::Running;
}

json WaitNode::ToJson() const {
    auto j = TreeNode::ToJson();
    j["waitTime"] = waitTime_.ToJson();
    return j;
}

void WaitNode::FromJson(const json& j) {
    TreeNode::FromJson(j);
    if (j.contains("waitTime")) {
        waitTime_.FromJson(j["waitTime"]);
    }
}

json WaitNode::SaveState() const 
{

    auto j = TreeNode::SaveState();
    j["elapsedTime"] = elapsedTime_;
    return j;
}

void WaitNode::LoadState(const json& j) {
    TreeNode::LoadState(j);
    if (j.contains("elapsedTime")) {
        elapsedTime_ = j["elapsedTime"].get<float>();
    }
}

PrintMessageNode::PrintMessageNode() : TreeNode("PrintMessage", "PrintMessageNode") {
    message_.SetConstantValue("Hello");
}

void PrintMessageNode::OnStart(BehaviorTreeContext& context)
{

    TreeNode::OnStart(context);

    std::string resolvedMessage = context.blackboard->ResolveBTVariable(message_, std::string(""));
    std::cout << resolvedMessage << std::endl;
}

NodeStatus PrintMessageNode::Execute(BehaviorTreeContext& context) {
    // Side-effect happened in OnStart already; complete immediately
    return NodeStatus::Success;
}

json PrintMessageNode::ToJson() const {
    auto j = TreeNode::ToJson();
    j["message"] = message_.ToJson();
    return j;
}

void PrintMessageNode::FromJson(const json& j) {
    TreeNode::FromJson(j);
    if (j.contains("message")) {
        message_.FromJson(j["message"]);
    }
}

ConditionalWaitNode::ConditionalWaitNode() : TreeNode("ConditionalWait", "ConditionalWaitNode") {
    waitTime_.SetConstantValue(1.0f);
    maxWaitTime_.SetConstantValue(5.0f);
    shouldWaitCondition_.SetConstantValue(true);
}

void ConditionalWaitNode::OnStart(BehaviorTreeContext& context) {
    elapsedTime_ = 0.0f;
}

NodeStatus ConditionalWaitNode::Execute(BehaviorTreeContext& context) {
    // Resolve all variables
    bool shouldWait = context.blackboard->ResolveBTVariable(shouldWaitCondition_, true);

    if (!shouldWait) {
        return NodeStatus::Success; // Don't wait if condition is false
    }

    float resolvedWaitTime = context.blackboard->ResolveBTVariable(waitTime_, 1.0f);
    float resolvedMaxWaitTime = context.blackboard->ResolveBTVariable(maxWaitTime_, 5.0f);

    elapsedTime_ += context.deltaTime;

    if (elapsedTime_ >= resolvedWaitTime) {
        return NodeStatus::Success;
    }

    // Safety: don't wait forever
    if (elapsedTime_ >= resolvedMaxWaitTime) {
        return NodeStatus::Failure;
    }

    return NodeStatus::Running;
}

json ConditionalWaitNode::ToJson() const {
    auto j = TreeNode::ToJson();
    j["waitTime"] = waitTime_.ToJson();
    j["maxWaitTime"] = maxWaitTime_.ToJson();
    j["shouldWaitCondition"] = shouldWaitCondition_.ToJson();
    return j;
}

void ConditionalWaitNode::FromJson(const json& j) {
    TreeNode::FromJson(j);
    if (j.contains("waitTime")) {
        waitTime_.FromJson(j["waitTime"]);
    }
    if (j.contains("maxWaitTime")) {
        maxWaitTime_.FromJson(j["maxWaitTime"]);
    }
    if (j.contains("shouldWaitCondition")) {
        shouldWaitCondition_.FromJson(j["shouldWaitCondition"]);
    }
}

json ConditionalWaitNode::SaveState() const {
    auto j = TreeNode::SaveState();
    j["elapsedTime"] = elapsedTime_;
    return j;
}

void ConditionalWaitNode::LoadState(const json& j) {
    TreeNode::LoadState(j);
    if (j.contains("elapsedTime")) {
        elapsedTime_ = j["elapsedTime"].get<float>();
    }
}