#pragma once

// The NPC's body as the brain sees it (NPC_AI_DESIGN.md §9): the running skill writes one BodyIntent per frame and the
// body applies it: path following, pace, facing, gaze and the weapon. Movement, animation, ragdoll and doors stay in
// NpcBase; this class only gives them their orders.

#include <MathHelper.hpp>

#include "NpcData.h"

#include <AiPerception/Observer.h>

class NpcBase;

enum class NpcGait
{
	Stand,
	Walk,
	Run
};

// How the gun is carried.
enum class NpcWeaponState
{
	Holstered,
	Held,    // pistol_hold: out, pointed down
	Ready,   // pistol_idle: up at the chest
	Aimed    // pistol_aim
};

// Where the body points.
enum class BodyFacing
{
	Path,    // the way it walks; standing still keeps the last facing
	Point    // keeps `facingPoint` in front, turning at the body's turn rate
};

struct BodyIntent
{
	// Movement
	bool move = false;
	vec3 destination = vec3();
	float acceptRadius = 0.5f;
	NpcGait gait = NpcGait::Walk;
	float speed = 0.0f;                  // m/s; 0 = the gait's pace from the profile

	// Facing and gaze
	BodyFacing facing = BodyFacing::Path;
	vec3 facingPoint = vec3();
	bool hasGaze = false;                // otherwise the head looks where the body faces
	vec3 gazePoint = vec3();
	bool glanceAllowed = true;           // a noticed stimulus may take the gaze for a moment (§9.2)

	NpcWeaponState weapon = NpcWeaponState::Holstered;

	bool crouch = false;                 // down on one knee (crouch_idle): looking after a body
};

class NpcBody
{
public:

	explicit NpcBody(NpcBase& owner) : npc(owner) {}

	void Setup(const NpcProfile* newProfile) { profile = newProfile; }

	// Call once the mesh stands in the world.
	void Start();

	// The head turns to a noticed point for the profile's glance time.
	void Glance(const vec3& point, double now);

	// Applies the intent for this frame (before NpcBase moves the body).
	void Apply(const BodyIntent& intent, float dt, double now);

	// The observer's eye and gaze for the next perception pass: the head, so the player can read where it looks.
	void UpdateObserver(Observer& observer, float visionRangeScale) const;

	// Gaze in degrees from the body's facing, for the animator.
	float GazeYaw() const { return gazeYaw; }
	float GazePitch() const { return gazePitch; }
	vec3 GazeDirection() const;
	vec3 EyePosition() const;

	// NpcBase's facing hook: true while the body keeps facing a point; `lookVector` is the way.
	bool FacingLock(vec3& lookVector) const;

	// The last destination was reached (never true in the first frames after a new destination).
	bool Arrived() const { return moving && settleFrames == 0 && reachedNow; }
	bool IsMoving() const { return moving; }

	bool Glancing(double now) const { return now < glanceUntil; }

	NpcWeaponState Weapon() const { return weapon; }

private:

	NpcBase& npc;
	const NpcProfile* profile = nullptr;

	bool moving = false;
	bool reachedNow = false;
	int settleFrames = 0;           // path results of the frame a destination was set are not trusted (the tree did the same)
	vec3 destination = vec3();

	bool facingLocked = false;
	vec3 facingDirection = vec3(0.0f, 0.0f, 1.0f); // flat; turns at the body's turn rate
	vec3 lockedLook = vec3();

	float gazeYaw = 0.0f;
	float gazePitch = 0.0f;

	vec3 glancePoint = vec3();
	double glanceUntil = -1.0;

	// What the head aims at this frame (a glance, else the intent's gaze) and whether it is beyond what the head can reach.
	bool hasGazeTarget = false;
	vec3 gazeTarget = vec3();
	bool gazeBeyondHead = false;

	NpcWeaponState weapon = NpcWeaponState::Holstered;

	float PaceFor(const BodyIntent& intent) const;
	void ApplyMovement(const BodyIntent& intent);
	void ApplyFacing(const BodyIntent& intent, float dt);
	void ApplyGaze(const BodyIntent& intent, float dt, double now);
};
