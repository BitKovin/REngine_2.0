#include <Entity.h>

#include "BehaviourTree/BehaviorTreeEditor.h"
#include <memory>

BehaviorTreeEditor editor;

class GameStart : public Entity
{
public:
    GameStart();
    ~GameStart();



    void Start()
    {
        editor.Init();
    }

    void Update()
    {
        editor.Update(Time::DeltaTimeF);

        this_thread::sleep_for(10ms);

    }

    void UpdateDebugUI()
    {
        editor.Draw();
    }

private:

};

GameStart::GameStart()
{
}

GameStart::~GameStart()
{
}

REGISTER_ENTITY(GameStart, "gamestart")