#pragma once
#include "TreeNode.h"

class CompositeNode : public TreeNode {
public:
    CompositeNode(const std::string& name, const std::string& type)
        : TreeNode(name, type)
    {
        isTask = false;
    }

    // Common composite interface
    virtual size_t GetCurrentChildIndex() const = 0;
    virtual void SetCurrentChildIndex(size_t index) = 0;

    // Make sure we can properly construct derived classes
    virtual ~CompositeNode() = default;
};