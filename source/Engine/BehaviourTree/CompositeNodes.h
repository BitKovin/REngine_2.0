#pragma once
#include "CompositeNode.h"

class SequenceNode : public CompositeNode {
public:
    SequenceNode();

    void OnStart(BehaviorTreeContext& context) override;
    void OnStop(BehaviorTreeContext& context) override;
    NodeStatus Execute(BehaviorTreeContext& context) override;
    void OnReset() override;
    void Abort(BehaviorTreeContext& context) override;

    size_t GetCurrentChildIndex() const override { return currentChildIndex_; }
    void SetCurrentChildIndex(size_t index) override { currentChildIndex_ = index; }

    json SaveState() const override;
    void LoadState(const json& j) override;

private:
    size_t currentChildIndex_ = 0;
};

//executes all children in same frame
class MultiExecSequenceNode : public CompositeNode {
public:
    MultiExecSequenceNode();

    void OnStart(BehaviorTreeContext& context) override;
    void OnStop(BehaviorTreeContext& context) override;
    NodeStatus Execute(BehaviorTreeContext& context) override;
    void OnReset() override;
    void Abort(BehaviorTreeContext& context) override;

    size_t GetCurrentChildIndex() const override { return currentChildIndex_; }
    void SetCurrentChildIndex(size_t index) override { currentChildIndex_ = index; }

    json SaveState() const override;
    void LoadState(const json& j) override;

private:
    size_t currentChildIndex_ = 0;
};

class SelectorNode : public CompositeNode {
public:
    SelectorNode();

    void OnStart(BehaviorTreeContext& context) override;
    NodeStatus Execute(BehaviorTreeContext& context) override;
    void OnReset() override;
    void Abort(BehaviorTreeContext& context) override;

    size_t GetCurrentChildIndex() const override { return currentChildIndex_; }
    void SetCurrentChildIndex(size_t index) override { currentChildIndex_ = index; }

    json SaveState() const override;
    void LoadState(const json& j) override;

private:
    size_t currentChildIndex_ = 0;
    int previousExecutedChild = -1;
};