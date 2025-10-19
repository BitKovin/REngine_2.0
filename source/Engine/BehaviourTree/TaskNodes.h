#pragma once
#include "TreeNode.h"
#include "BTVariable.h"
#include <string>
#include "../json.hpp"

using nlohmann::json;

class WaitNode : public TreeNode {
public:
    WaitNode();

    void OnStart(BehaviorTreeContext& context) override;
    NodeStatus Execute(BehaviorTreeContext& context) override;

    // Setters for different value types
    void SetWaitTime(float time) { waitTime_.SetConstantValue(time); }
    void SetWaitTime(const BTVariable& var) { waitTime_ = var; }
    void BindWaitTimeToBlackboard(const std::string& key) {
        waitTime_ = BTVariable::FromBlackboard(key);
    }

    float GetWaitTime() const {
        if (waitTime_.GetSourceType() == BTVariable::SourceType::Constant) {
            try {
                return std::get<float>(waitTime_.GetConstantValue());
            }
            catch (const std::bad_variant_access&) {
                return 1.0f;
            }
        }
        return 1.0f;
    }

    const BTVariable& GetWaitTimeVariable() const { return waitTime_; }

    json ToJson() const override;
    void FromJson(const json& j) override;

    json SaveState() const override;
    void LoadState(const json& j) override;

private:
    BTVariable waitTime_;
    float elapsedTime_ = 0.0f;
};

class PrintMessageNode : public TreeNode {
public:
    PrintMessageNode();

    void OnStart(BehaviorTreeContext& context) override;
    NodeStatus Execute(BehaviorTreeContext& context) override;

    //void SetMessage(const std::string& message) { message_.SetConstantValue(message); }
    void SetMessage(const BTVariable& var) { message_ = var; }
    void BindMessageToBlackboard(const std::string& key) {
        message_ = BTVariable::FromBlackboard(key);
    }

    const BTVariable& GetMessageVariable() const { return message_; }

    json ToJson() const override;
    void FromJson(const json& j) override;

private:
    BTVariable message_;
};

class ConditionalWaitNode : public TreeNode {
public:
    ConditionalWaitNode();

    void OnStart(BehaviorTreeContext& context) override;
    NodeStatus Execute(BehaviorTreeContext& context) override;

    void SetWaitTime(const BTVariable& var) { waitTime_ = var; }
    void SetMaxWaitTime(const BTVariable& var) { maxWaitTime_ = var; }
    void SetShouldWaitCondition(const BTVariable& var) { shouldWaitCondition_ = var; }

    void BindWaitTimeToBlackboard(const std::string& key) {
        waitTime_ = BTVariable::FromBlackboard(key);
    }
    void BindMaxWaitTimeToBlackboard(const std::string& key) {
        maxWaitTime_ = BTVariable::FromBlackboard(key);
    }
    void BindShouldWaitConditionToBlackboard(const std::string& key) {
        shouldWaitCondition_ = BTVariable::FromBlackboard(key);
    }

    const BTVariable& GetWaitTimeVariable() const { return waitTime_; }
    const BTVariable& GetMaxWaitTimeVariable() const { return maxWaitTime_; }
    const BTVariable& GetShouldWaitConditionVariable() const { return shouldWaitCondition_; }

    json ToJson() const override;
    void FromJson(const json& j) override;

    json SaveState() const override;
    void LoadState(const json& j) override;

private:
    BTVariable waitTime_;
    BTVariable maxWaitTime_;
    BTVariable shouldWaitCondition_;
    float elapsedTime_ = 0.0f;
};