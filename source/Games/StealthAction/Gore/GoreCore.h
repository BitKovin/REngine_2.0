#pragma once

#include "GoreTriGrid.h"

#include <skinned_model.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace Gore
{
    // The "core": one universal human body without clothes (GameData/models/npc/goreHumanCore.glb) that closes the holes of every
    // humanoid. The holes of a character are cut in its own mesh as before; what is generated inside them (the wound cavities,
    // GoreCavity.h) is made against the core, which has the same bone names, so it follows the character's bones. It does not fit
    // every character exactly: where the clothes are thicker the hole shows no filling, where the core would reach past the clothes
    // the cavity is clipped by the character's own body (GoreCavity.cpp). Cuts (a lost limb, a head) do not use it.
    //
    // The core is moved bone by bone onto the character: each core bone that the character has (by name, after the profile's
    // "boneMap") is placed on that bone and scaled by the ratio of the two bones' lengths, so the limbs follow the character's
    // proportions in the bind pose. Bones the character lacks follow their nearest ancestor that it has.
    //
    // Returns the core as a grid in the character's bind space. Its bone weights are the character's own skin indices. Runs on the
    // worker thread, reads both models only. Null when no bone of the core matches.
    std::shared_ptr<const TriGrid> BuildFittedCore(const roj::SkinnedModel& core, const roj::SkinnedModel& body,
                                                   const std::unordered_map<std::string, std::string>& boneMap);
}
