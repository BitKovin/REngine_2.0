#pragma once

#include "CustomTask.h"

class HoldTask : public CustomTask
{
public:
	HoldTask();
	~HoldTask();

	void Start(BehaviorTreeContext& context) override;
	void Tick(BehaviorTreeContext& context) override;

private:

};



