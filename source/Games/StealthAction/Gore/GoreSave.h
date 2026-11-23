#pragma once

#include <json.hpp>

class Entity;

namespace Gore
{
    // Save games. The gore state of a body (its wounds, cuts, blood decals and the damage collected towards severing a part) is
    // written into the save record of the entity as target["gore"]. Nothing is written for a body without gore, and severed
    // parts (GoreGib) are not saved: after a load the body still shows the stump, the part itself is gone.
    //
    // Times are saved relative to now (the regen clock for wounds and cuts, game time for the effects), so a wound that was half
    // healed continues where it was. The geometry that is generated from the state is not saved: the cut faces and the wound
    // cavities are built again on the worker thread after the load, and the cut parts' hitboxes are switched off again, both by
    // the gore tick as soon as the model is ready (GoreSystem.cpp, RestoreLoadedGore).
    void SaveGoreState(Entity* entity, nlohmann::json& target);

    // Reads target["gore"] back into the entity's GoreComponent (added when missing). Does nothing when there is none.
    void LoadGoreState(Entity* entity, const nlohmann::json& source);
}
