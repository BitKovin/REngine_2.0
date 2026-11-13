#pragma once

// Development builds only: renders weapon icons for the HUD weapon grid.
//
// For every row of GameData/tables/weapons/ whose icon is missing or older than
// its model, the weapon model is drawn into an off-screen render target with
// the renderer's own draw commands (orthographic side view, fixed lighting),
// read back with Screenshot::RequestTarget, recoloured to the HUD's bone palette
// with a dark outline, and written as GameData/textures/ui/weapon_icons/<class>.png
// into both the game's source GameData (to be committed) and the running copy.
//
// Spawned by Player::Start(). Console: weapon_icons.rebake [className].

#ifndef DISTRIBUTION

#include <Entity.h>

class WeaponIconBaker : public Entity
{
public:

	// Spawns the baker into the current level once per run (or again after a
	// rebake request). Does nothing when every icon is up to date.
	static void StartIfNeeded();

	// Forces a rebake: of one class, or of every class when className is empty.
	static void RequestRebake(const std::string& className);

	WeaponIconBaker();

	void Start() override;
	void Update() override;
	void Destroy() override;

private:

	class Impl;
	Impl* impl = nullptr;

};

#endif
