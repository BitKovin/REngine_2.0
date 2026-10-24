#pragma once

// The walking pawn (SLICE_M2B_BRIDGE.md 3.4): the player as far as the sim is concerned. A deliberately small Entity on the
// engine's CharacterController: mouse look, the movement actions GameStart registers (forward, backward, left, right, crouch,
// jump, view), an eye camera. No weapons, interaction or UI. Created by SimSpawn at the level's default player start.
//
// It is the driver of a character (SLICE_M2F_CHARACTERS.md): every frame it fills a CharacterState and applies it to its CharacterBody (the player's
// own third-person look, which in first person is the body seen from its eyes: no neck, no arms, the full body in the shadow pass).
// View modes: first person, and third person with a camera boom behind the pawn (the V key, console `sim_view`, launch `-sim_view`).
//
// Speed comes from the shared speed table (SimSpeedTable.h): the run speed by default, the walk speed while crouching, so the
// pawn and a walking agent cover ground at the same pace.

#include <Characters/CharacterBody.h>

#include <Character/CharacterController.h>
#include <Entity.h>
#include <EntityHandle.h>

#include <optional>

class SimPawn : public Entity
{
public:
	enum class ViewMode : uint8_t
	{
		FirstPerson,
		ThirdPerson,
	};

	SimPawn();
	~SimPawn() override;

	// The pawn of the loaded level, if any.
	static inline SimPawn* Instance = nullptr;
	// False while the baker or the self tests load levels: no pawn then.
	static inline bool SpawnEnabled = true;
	// The player's chosen view: it outlives pawns, so a level transition keeps it (launch -sim_view sets it). -sim_pitch: a pitch (degrees, positive
	// looks down) the first pawn starts with, for pictures.
	static inline ViewMode View = ViewMode::FirstPerson;
	static inline std::optional<float> LaunchPitch;

	// Creates the pawn at a spawn point (feet position, yaw and the view's pitch in degrees). Does nothing when spawning is disabled or this
	// level has one.
	static SimPawn* SpawnAt(const glm::vec3& feet, float yaw, float pitch = 0.0f);

	void Start() override;
	void Update() override;
	void Destroy() override;
	void Teleport(vec3 target) override;
	void Serialize(json& target);
	void Deserialize(json& source);

	// Where the pawn stands (the floor under the capsule).
	glm::vec3 FeetPosition() const;
	// The view's pitch in degrees (what a level transition carries over).
	float Pitch() const { return cameraRotation.x; }

	static void SetViewMode(ViewMode mode) { View = mode; }
	static ViewMode GetViewMode() { return View; }
	// The player's own CharacterBody (null until its first frame in a level).
	CharacterBody* Body() const;

	// Turns the view to a point (a camera for looking at something: sim_look, -sim_look).
	void LookAt(const glm::vec3& point);

	// Automated runs: walk forward, turn away from walls, until `metres` of ground have been covered.
	void StartScriptedWalk(float metres);
	bool ScriptedWalkDone() const { return scriptedTarget > 0.0f && scriptedWalked >= scriptedTarget; }
	float ScriptedWalked() const { return scriptedWalked; }

private:
	CharacterController controller;
	EntityHandle bodyHandle;
	bool bodyCreated = false;
	vec3 cameraRotation = vec3(0.0f);
	vec3 velocity = vec3(0.0f);

	float scriptedTarget = 0.0f;
	float scriptedWalked = 0.0f;
	float scriptedStuck = 0.0f;
	vec3 scriptedLast = vec3(0.0f);

	CharacterState MakeState(bool crouching);
	void PlaceThirdPersonCamera();
};
