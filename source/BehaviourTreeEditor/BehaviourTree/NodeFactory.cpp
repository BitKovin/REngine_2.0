#include "NodeFactory.h"
#include "CompositeNodes.h"
#include "DecoratorNodes.h"
#include "TaskNodes.h"

NodeFactory& NodeFactory::GetInstance() {
    static NodeFactory instance;
    return instance;
}

NodeFactory::NodeFactory() {
    // Register default node types
    RegisterNode<SequenceNode>("SequenceNode");
    RegisterNode<SelectorNode>("SelectorNode");
    RegisterNode<WaitNode>("WaitNode");
    RegisterNode<PrintMessageNode>("PrintMessageNode");
    RegisterNode<ConditionalWaitNode>("ConditionalWaitNode");
    RegisterNode<InverterDecorator>("InverterDecorator");
    RegisterNode<RepeatDecorator>("RepeatDecorator");
    RegisterNode<SucceederDecorator>("SucceederDecorator");
    RegisterNode<FailerDecorator>("FailerDecorator");
    RegisterNode<ConditionalDecorator>("ConditionalDecorator");
}

std::shared_ptr<TreeNode> NodeFactory::CreateNode(const std::string& typeName) const {
    auto it = creators_.find(typeName);
    if (it != creators_.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> NodeFactory::GetRegisteredNodeTypes() const {
    std::vector<std::string> types;
    for (const auto& pair : creators_) {
        types.push_back(pair.first);
    }
    return types;
}

// Register nodes
REGISTER_BT_NODE(SequenceNode)
REGISTER_BT_NODE(SelectorNode)
REGISTER_BT_NODE(WaitNode)
REGISTER_BT_NODE(PrintMessageNode)
REGISTER_BT_NODE(ConditionalWaitNode)
REGISTER_BT_NODE(InverterDecorator)
REGISTER_BT_NODE(RepeatDecorator)
REGISTER_BT_NODE(SucceederDecorator)
REGISTER_BT_NODE(FailerDecorator)
REGISTER_BT_NODE(ConditionalDecorator)