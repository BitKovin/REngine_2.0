#include "TaskPhasedAction.h"
#include "../NpcBase.h"

class Task_ChairPoint : public TaskPhasedAction
{
public:
    Task_ChairPoint()
    {
        enterAnim = "sit_down";  enterDur = 2.6f;
        loopAnim = "sitting_idle"; loopDur = 8.0f;
        exitAnim = "stand_up";  exitDur = 2.6f;
    }
};

REGISTER_ENTITY(Task_ChairPoint, "task_chair")