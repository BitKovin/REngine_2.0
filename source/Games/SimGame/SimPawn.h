#pragma once

// The walking pawn (SLICE_M2B_BRIDGE.md 3.4): the player as far as the sim is concerned. A deliberately small Entity on the
// engine's CharacterController: mouse look, the movement actions GameStart registers (forward, backward, left, right, crouch,
// jump), an eye camera. No weapons, interaction, animation or UI. Created by SimSpawn at the level's default player start.
//
// Speed comes from the shared speed table (SimSpeedTable.h): the run speed by default, the walk speed while crouching, so the
// pawn and a walking agent cover ground at the same pace.

#include <Character/CharacterController.h>
#include <Entity.h>

class SimPawn : public Entity
{
public:
	SimPawn();
	~SimPawn() override;

	// The pawn of the loaded level, if any.
	static inline SimPawn* Instance = nullptr;
	// False while the baker or the self tests load levels: no pawn then.
	static inline bool SpawnEnabled = true;

	// Creates the pawn at a spawn point (feet position and yaw). Does nothing when spawning is disabled or this level has one.
	static SimPawn* SpawnAt(const glm::vec3& feet, float yaw);

	void Start() override;
	void Update() override;
	void Teleport(vec3 target) override;
	void Serialize(json& target);
	void Deserialize(json& source);

	// Where the pawn stands (the floor under the capsule).
	glm::vec3 FeetPosition() const;

	// Turns the view to a point (a camera for looking at something: sim_look, -sim_look).
	void LookAt(const glm::vec3& point);

	// Automated runs: walk forward, turn away from walls, until `metres` of ground have been covered.
	void StartScriptedWalk(float metres);
	bool ScriptedWalkDone() const { return scriptedTarget > 0.0f && scriptedWalked >= scriptedTarget; }
	float ScriptedWalked() const { return scriptedWalked; }

private:
	CharacterController controller;
	vec3 cameraRotation = vec3(0.0f);
	vec3 velocity = vec3(0.0f);

	float scriptedTarget = 0.0f;
	float scriptedWalked = 0.0f;
	float scriptedStuck = 0.0f;
	vec3 scriptedLast = vec3(0.0f);
};
