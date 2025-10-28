#include "CustomTask.h"


CustomTask::CustomTask(const std::string& name, const std::string& type) : TreeNode(name, type)
{
}


inline void CustomTask::OnStart(BehaviorTreeContext& context)
{

	lastContext = context;

	TreeNode::OnStart(context);

	Start(context);
}

inline NodeStatus CustomTask::Execute(BehaviorTreeContext& context)
{

	Tick(context);

	return GetStatus();
}

void CustomTask::FinishExecution(bool success)
{
	SetStatus(success ? NodeStatus::Success : NodeStatus::Failure);
}

json CustomTask::ToJson() const
{

	auto j = TreeNode::ToJson();

	for (auto& var : Variables)
	{
		j[var.first] = var.second.ToJson();
	}

	return j;
}

void CustomTask::FromJson(const json& j)
{

	TreeNode::FromJson(j);

	for (auto& var : Variables)
	{
		if (j.contains(var.first))
		{
			var.second.FromJson(j.at(var.first));
		}
	}

}
