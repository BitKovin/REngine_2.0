#pragma once

#include "TreeNode.h"
#include "BTVariable.h"

#include <unordered_map>

class CustomTask : public TreeNode
{
public:
	CustomTask(const std::string& name, const std::string& type);

	void OnStart(BehaviorTreeContext& context) override;
	NodeStatus Execute(BehaviorTreeContext& context) override;

	void FinishExecution(bool success = false);

	virtual void Tick(BehaviorTreeContext& context){}
	virtual void Start(BehaviorTreeContext& context) {}

	json ToJson() const override;
	void FromJson(const json& j) override;

	std::unordered_map<std::string, BTVariable> Variables;

	template<typename T>
	T GetVariable(const std::string& name, const T& defaultValue = T{}) const {
		auto it = Variables.find(name);
		if (it == Variables.end()) return defaultValue;

		T value{};
		return it->second.Resolve(lastContext.blackboard, value) ? value : defaultValue;
	}

private:

	BehaviorTreeContext lastContext;

};
