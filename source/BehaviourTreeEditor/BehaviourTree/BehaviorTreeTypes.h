#pragma once
#include <json.hpp>

using nlohmann::json;

enum class NodeStatus {
    Idle,
    Running,
    Success,
    Failure
};

// Forward declaration
class Blackboard;
class BehaviorTree;

struct BehaviorTreeContext {
    Blackboard* blackboard;
    float deltaTime;
    BehaviorTree* tree;
    bool reachedTask;
};

enum class FlowAbortMode {
    None,           // Never aborts
    Self,           // Aborts self (the branch below the decorator)
    LowerPriority,  // Aborts lower priority branches
    Both            // Aborts both self and lower priority
};