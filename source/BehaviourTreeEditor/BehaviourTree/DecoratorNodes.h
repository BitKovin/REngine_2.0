#pragma once
#include "TreeNode.h"
#include "BTVariable.h"
#include <string>
#include <vector>
#include <json.hpp>

using nlohmann::json;

class DecoratorNode : public TreeNode {
public:
    DecoratorNode(const std::string& name, const std::string& type);

    void SetChild(std::shared_ptr<TreeNode> child);
    std::shared_ptr<TreeNode> GetChild() const;

    void SetAbortMode(FlowAbortMode mode) { abortMode_ = mode; }
    FlowAbortMode GetAbortMode() const { return abortMode_; }

    void AddObserverKey(const std::string& key) { observerKeys_.push_back(key); }
    const std::vector<std::string>& GetObserverKeys() const { return observerKeys_; }

    virtual bool CheckCondition(BehaviorTreeContext& context) const = 0;

protected:
    FlowAbortMode abortMode_ = FlowAbortMode::None;
    std::vector<std::string> observerKeys_;
};

class InverterDecorator : public DecoratorNode {
public:
    InverterDecorator();
    NodeStatus Execute(BehaviorTreeContext& context) override;
    bool CheckCondition(BehaviorTreeContext& context) const override { return true; }
};

class RepeatDecorator : public DecoratorNode {
public:
    RepeatDecorator();

    void OnStart(BehaviorTreeContext& context) override;
    NodeStatus Execute(BehaviorTreeContext& context) override;
    bool CheckCondition(BehaviorTreeContext& context) const override { return true; }

    void SetRepeatCount(int count) { repeatCount_.SetConstantValue(count); }
    void SetRepeatCount(const BTVariable& var) { repeatCount_ = var; }
    void BindRepeatCountToBlackboard(const std::string& key) {
        repeatCount_ = BTVariable::FromBlackboard(key);
    }

    const BTVariable& GetRepeatCountVariable() const { return repeatCount_; }

    json ToJson() const override;
    void FromJson(const json& j) override;

    json SaveState() const override;
    void LoadState(const json& j) override;

private:
    BTVariable repeatCount_;
    int currentCount_ = 0;
};

class SucceederDecorator : public DecoratorNode {
public:
    SucceederDecorator();
    NodeStatus Execute(BehaviorTreeContext& context) override;
    bool CheckCondition(BehaviorTreeContext& context) const override { return true; }
};

class FailerDecorator : public DecoratorNode {
public:
    FailerDecorator();
    NodeStatus Execute(BehaviorTreeContext& context) override;
    bool CheckCondition(BehaviorTreeContext& context) const override { return true; }
};

// New decorator with abort capability
class ConditionalDecorator : public DecoratorNode {
public:
    ConditionalDecorator();

    NodeStatus Execute(BehaviorTreeContext& context) override;
    bool CheckCondition(BehaviorTreeContext& context) const override;

    void SetCondition(const BTVariable& condition) { condition_ = condition; }
    void BindConditionToBlackboard(const std::string& key) {
        condition_ = BTVariable::FromBlackboard(key);
        AddObserverKey(key); // Watch this key for abort
    }

    const BTVariable& GetConditionVariable() const { return condition_; }

    json ToJson() const override;
    void FromJson(const json& j) override;

    bool inverse = false;

    bool previousCondition = false;

private:
    BTVariable condition_;
};